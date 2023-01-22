import Konva from 'konva';
import { TextConfig } from 'konva/lib/shapes/Text';
import { Block } from './Block';
import { Group, Rect, Text } from 'react-konva';
import { RenderProps } from './RenderBlock';
import { createContext, useCallback, useContext, useState } from 'react';
import { setCursor } from 'components/Konva';

export type TextStyleType = Omit<TextConfig, 'text'> & {
  fill?: string;
};

type IndexType = number[];

const indexInRange = (idx: IndexType, start: IndexType, end: IndexType): boolean => {
  return (idx >= start && idx <= end) || (idx >= end && idx <= start);
};

export type TextSelectionType = {
  area?: string;
  index?: IndexType;
};

interface TextSelectionContextValue {
  selectedArea: string;
  setSelectedArea: (area: string) => void;
  firstSelectedIdx?: IndexType;
  setFirstSelectedIdx: (idx?: IndexType) => void;
  lastSelectedIdx?: IndexType;
  setLastSelectedIdx: (idx?: IndexType) => void;
  selecting?: boolean;
  setSelecting: (selecting: boolean) => void;
  clearSelection: () => void;
  stopSelecting: () => void;
}

const TextSelectionContext = createContext<TextSelectionContextValue | null>(null);

export interface TextSelectionProviderProps {
  children: React.ReactNode;
}

export const TextSelectionProvider = ({ children }: TextSelectionProviderProps) => {
  const [selectedArea, setSelectedArea] = useState<string>('default');
  const [firstSelectedIdx, setFirstSelectedIdx] = useState<IndexType | undefined>();
  const [lastSelectedIdx, setLastSelectedIdx] = useState<IndexType | undefined>();
  const [selecting, setSelecting] = useState(false);

  const clearSelection = useCallback(() => {
    setFirstSelectedIdx(undefined);
    setLastSelectedIdx(undefined);
    setSelecting(false);
  }, [setFirstSelectedIdx, setLastSelectedIdx, setSelecting]);

  const stopSelecting = useCallback(() => {
    if (selecting) {
      setSelecting(false);
    } else {
      clearSelection();
    }
  }, [selecting, setSelecting, clearSelection]);

  return (
    <TextSelectionContext.Provider
      value={{
        selectedArea,
        setSelectedArea,
        firstSelectedIdx,
        setFirstSelectedIdx,
        lastSelectedIdx,
        setLastSelectedIdx,
        selecting,
        setSelecting,
        clearSelection,
        stopSelecting,
      }}
    >
      {children}
    </TextSelectionContext.Provider>
  );
};

export function useTextSelection() {
  const context = useContext(TextSelectionContext);
  if (!context) {
    throw new Error('useTextSelection must be used within a TextSelectionProvider');
  }
  return context;
}

export const TextSelectionBridgeProvider = TextSelectionContext.Provider;
export const TextSelectionBridgeConsumer = TextSelectionContext.Consumer;

export type TextBlockProps = TextStyleType & {
  text: string;
  ctx?: {
    textStyle: TextStyleType;
    textSelection: TextSelectionType;
  };
};

export const StaticTextBlock = ({ text, ctx, ...propsStyle }: TextBlockProps) => {
  const selArea = ctx?.textSelection?.area || 'default';
  let selIndex = ctx?.textSelection?.index || [];
  if (selIndex.length > 0) {
    selIndex[selIndex.length - 1]++;
    selIndex = [...selIndex];
    text = `<${selIndex.join(',')}>${text}`;
  }
  const style = { ...ctx?.textStyle, ...propsStyle };
  const konvaText = new Konva.Text(style);
  const textSize = konvaText.measureSize(text);
  const TextRender = (props: RenderProps) => {
    const {
      selectedArea,
      setSelectedArea,
      firstSelectedIdx,
      setFirstSelectedIdx,
      lastSelectedIdx,
      setLastSelectedIdx,
      selecting,
      setSelecting,
    } = useTextSelection();

    const isSelected =
      firstSelectedIdx &&
      lastSelectedIdx &&
      selectedArea === selArea &&
      indexInRange(selIndex, firstSelectedIdx, lastSelectedIdx);

    const onMouseDown = useCallback(() => {
      setSelectedArea?.(selArea);
      setFirstSelectedIdx?.(selIndex);
      setLastSelectedIdx?.(undefined);
      setSelecting?.(true);
    }, [setFirstSelectedIdx, setSelecting]);

    const onMouseMove = useCallback(() => {
      if (selecting) {
        setLastSelectedIdx?.(selIndex);
      }
    }, [selecting, setLastSelectedIdx]);

    const onMouseEnter = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      setCursor(e, 'text');
      console.log(selIndex);
    }, []);

    const onMouseLeave = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      setCursor(e, 'default');
    }, []);

    return (
      <Group
        {...props}
        onMouseDown={onMouseDown}
        onMouseMove={onMouseMove}
        onMouseEnter={onMouseEnter}
        onMouseLeave={onMouseLeave}
      >
        <Rect
          width={props.width}
          height={props.height}
          fill={isSelected ? 'green' : undefined}
        ></Rect>
        <Text text={text} {...style} />
      </Group>
    );
  };
  return <Block width={textSize.width} height={textSize.height} render={<TextRender />} />;
};
