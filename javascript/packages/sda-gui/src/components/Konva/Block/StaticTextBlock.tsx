import Konva from 'konva';
import { TextConfig } from 'konva/lib/shapes/Text';
import { Block } from './Block';
import { Group, Rect, Text } from 'react-konva';
import { RenderProps } from './RenderBlock';
import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
} from 'react';
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

export const toSelIndexFromRenderProps = (
  { absX, absY, textSelection }: RenderProps,
  ctx?: {
    textSelection: TextSelectionType;
  },
): SelIndexType => {
  const selX = (absX || 0) - (textSelection?.startPointX || 0);
  const selY = (absY || 0) - (textSelection?.startPointY || 0);
  return toSelIndex(ctx?.textSelection?.index, selX, selY);
};

const selIndexInRange = (idx: SelIndexType, start: SelIndexType, end: SelIndexType): boolean => {
  return (idx >= start && idx <= end) || (idx >= end && idx <= start);
};

export type TextSelectionType = {
  area?: string;
  index?: number;
};

type SelectionObject = unknown;

interface TextSelectionContextValue {
  selectedText: string;
  selectedObjects: SelectionObject[];
  selectedAreaType: string;
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
  selectionContains: (selIndex: SelIndexType, selArea?: string) => boolean;
  addTokenToSelection: (selIndex: SelIndexType, token: string) => void;
  addObjectToSelection: (selIndex: SelIndexType, object: SelectionObject, key?: number) => void;
}

const TextSelectionContext = createContext<TextSelectionContextValue | null>(null);

export interface TextSelectionProviderProps {
  children: React.ReactNode;
}

export const TextSelectionProvider = ({ children }: TextSelectionProviderProps) => {
  const mapOfTokens = useRef<Map<SelIndexType, string>>(new Map());
  const mapOfObjects = useRef<Map<SelIndexType, SelectionObject>>(new Map());
  const setOfObjectKeys = useRef<Set<number>>(new Set());
  const [selectedText, setSelectedText] = useState<string>('');
  const [selectedObjects, setSelectedObjects] = useState<SelectionObject[]>([]);
  const [selectedArea, setSelectedArea] = useState<string>('default');
  const [firstSelectedIdx, setFirstSelectedIdx] = useState<SelIndexType | undefined>();
  const [lastSelectedIdx, setLastSelectedIdx] = useState<SelIndexType | undefined>();
  const [selecting, setSelecting] = useState(false);

  const selectedAreaType = useMemo(() => selectedArea.split('-').reverse()[0], [selectedArea]);

  const selectionMapToList = (selection: Map<SelIndexType, unknown>) => {
    if (firstSelectedIdx === undefined || lastSelectedIdx === undefined) return [];
    const objects = Array.from(selection.entries())
      .filter(([idx]) => selIndexInRange(idx, firstSelectedIdx, lastSelectedIdx))
      .sort(([idx1], [idx2]) => (idx1 > idx2 ? 1 : -1))
      .map(([, object]) => object);
    return objects;
  };

  const extractSelectedText = useCallback(() => {
    const tokens = selectionMapToList(mapOfTokens.current);
    const text = tokens.join('');
    setSelectedText(text);
    mapOfTokens.current.clear();
  }, [selectionMapToList, mapOfTokens]);

  const extractSelectedObjects = useCallback(() => {
    const objects = selectionMapToList(mapOfObjects.current);
    setSelectedObjects(objects);
    mapOfObjects.current.clear();
    setOfObjectKeys.current.clear();
  }, [selectionMapToList, mapOfObjects]);

  const clearSelection = useCallback(() => {
    setFirstSelectedIdx(undefined);
    setLastSelectedIdx(undefined);
    setSelecting(false);
  }, [setFirstSelectedIdx, setLastSelectedIdx, setSelecting]);

  const stopSelecting = useCallback(() => {
    if (selecting) {
      setSelecting(false);
      extractSelectedText();
      extractSelectedObjects();
    } else {
      clearSelection();
    }
  }, [selecting, setSelecting, extractSelectedText, extractSelectedObjects, clearSelection]);

  const selectionContains = useCallback(
    (selIndex: SelIndexType, selArea?: string) =>
      Boolean(
        firstSelectedIdx &&
          lastSelectedIdx &&
          selectedArea === (selArea || 'default') &&
          selIndexInRange(selIndex, firstSelectedIdx, lastSelectedIdx),
      ),
    [firstSelectedIdx, lastSelectedIdx, selectedArea],
  );

  const addTokenToSelection = useCallback(
    (selIndex: SelIndexType, token: string) => {
      mapOfTokens.current.set(selIndex, token);
    },
    [mapOfTokens],
  );

  const addObjectToSelection = useCallback(
    (selIndex: SelIndexType, object: SelectionObject, key?: number) => {
      if (key !== undefined) {
        // key is used to prevent adding the same object multiple times
        if (setOfObjectKeys.current.has(key)) return;
        setOfObjectKeys.current.add(key);
      }
      mapOfObjects.current.set(selIndex, object);
    },
    [mapOfObjects, setOfObjectKeys],
  );

  return (
    <TextSelectionContext.Provider
      value={{
        selectedText,
        selectedObjects,
        selectedAreaType,
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
        selectionContains,
        addTokenToSelection,
        addObjectToSelection,
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
      setSelectedArea,
      setFirstSelectedIdx,
      setLastSelectedIdx,
      selecting,
      setSelecting,
      selectionContains,
      addTokenToSelection,
    } = useTextSelection();
    const selIndex = toSelIndexFromRenderProps(props, ctx);
    const isSelected = selectionContains(selIndex, selArea);

    useEffect(() => {
      if (isSelected) {
        addTokenToSelection(selIndex, text);
      }
    }, [isSelected]);

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
        {isSelected && <Rect width={props.width} height={props.height} fill="green"></Rect>}
        <Text text={text} {...style} />
      </Group>
    );
  };
  return <Block width={textSize.width} height={textSize.height} render={<TextRender />} />;
};
