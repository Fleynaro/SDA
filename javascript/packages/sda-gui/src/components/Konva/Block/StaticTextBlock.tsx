import Konva from 'konva';
import { TextConfig } from 'konva/lib/shapes/Text';
import { Block } from './Block';
import { Group, Rect, Text } from 'react-konva';
import { RenderProps } from './RenderBlock';
import { createContext, useCallback, useContext, useEffect, useRef, useState } from 'react';
import { setCursor } from 'components/Konva';

export type TextStyleType = Omit<TextConfig, 'text'> & {
  fill?: string;
};

type SelIndexType = bigint;

export const toSelIndex = (index?: number, x?: number, y?: number): SelIndexType => {
  const indexBn = BigInt(index || 0);
  const xBn = BigInt((x && Math.floor(x)) || 0);
  const yBn = BigInt((y && Math.floor(y)) || 0);
  const maxSize = BigInt(1000000);
  return indexBn * maxSize * maxSize + yBn * maxSize + xBn;
};

const selIndexInRange = (idx: SelIndexType, start: SelIndexType, end: SelIndexType): boolean => {
  return (idx >= start && idx <= end) || (idx >= end && idx <= start);
};

export type TextSelectionType = {
  area?: string;
  index?: number;
  setStartPointHere?: boolean;
};

interface TextSelectionContextValue {
  selectionRef: React.MutableRefObject<Map<SelIndexType, string>>;
  selectedText: string;
  selectedArea: string;
  setSelectedArea: (area: string) => void;
  firstSelectedIdx?: SelIndexType;
  setFirstSelectedIdx: (idx?: SelIndexType) => void;
  lastSelectedIdx?: SelIndexType;
  setLastSelectedIdx: (idx?: SelIndexType) => void;
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
  const selectionRef = useRef<Map<SelIndexType, string>>(new Map());
  const [selectedText, setSelectedText] = useState<string>('');
  const [selectedArea, setSelectedArea] = useState<string>('default');
  const [firstSelectedIdx, setFirstSelectedIdx] = useState<SelIndexType | undefined>();
  const [lastSelectedIdx, setLastSelectedIdx] = useState<SelIndexType | undefined>();
  const [selecting, setSelecting] = useState(false);

  const extractSelectedText = useCallback(() => {
    if (firstSelectedIdx === undefined || lastSelectedIdx === undefined) return;
    const tokens = Array.from(selectionRef.current.entries())
      .filter(([idx]) => selIndexInRange(idx, firstSelectedIdx, lastSelectedIdx))
      .sort(([idx1], [idx2]) => (idx1 > idx2 ? 1 : -1));
    const text = tokens.map(([, token]) => token).join('');
    setSelectedText(text);
    selectionRef.current.clear();
  }, [firstSelectedIdx, lastSelectedIdx, selectionRef]);

  const clearSelection = useCallback(() => {
    setFirstSelectedIdx(undefined);
    setLastSelectedIdx(undefined);
    setSelecting(false);
  }, [setFirstSelectedIdx, setLastSelectedIdx, setSelecting]);

  const stopSelecting = useCallback(() => {
    if (selecting) {
      setSelecting(false);
      extractSelectedText();
    } else {
      clearSelection();
    }
  }, [selecting, setSelecting, extractSelectedText, clearSelection]);

  return (
    <TextSelectionContext.Provider
      value={{
        selectionRef,
        selectedText,
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
  const style = { ...ctx?.textStyle, ...propsStyle };
  const konvaText = new Konva.Text(style);
  const textSize = konvaText.measureSize(text);
  const TextRender = (props: RenderProps) => {
    const {
      selectionRef,
      selectedArea,
      setSelectedArea,
      firstSelectedIdx,
      setFirstSelectedIdx,
      lastSelectedIdx,
      setLastSelectedIdx,
      selecting,
      setSelecting,
    } = useTextSelection();

    const selX = (props.absX || 0) - (props.textSelection?.startPointX || 0);
    const selY = (props.absY || 0) - (props.textSelection?.startPointY || 0);
    const selIndex = toSelIndex(ctx?.textSelection?.index, selX, selY);

    const isSelected =
      firstSelectedIdx &&
      lastSelectedIdx &&
      selectedArea === selArea &&
      selIndexInRange(selIndex, firstSelectedIdx, lastSelectedIdx);

    useEffect(() => {
      if (isSelected) {
        selectionRef.current.set(selIndex, text);
      }
    }, [firstSelectedIdx, lastSelectedIdx]);

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
      // console.log('idx', ctx?.textSelection?.index, 'absX', props.absX, 'absY', props.absY);
    }, [selecting, setLastSelectedIdx]);

    const onMouseEnter = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      setCursor(e, 'text');
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
        <Text text={text} {...style} listening={false} />
      </Group>
    );
  };
  return <Block width={textSize.width} height={textSize.height} render={<TextRender />} />;
};
