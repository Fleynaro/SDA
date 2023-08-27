import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
} from 'react';
import { RenderProps } from './RenderBlock';

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
    textSelection: KonvaTextSelectionType;
  },
): SelIndexType => {
  const selX = (absX || 0) - (textSelection?.startPointX || 0);
  const selY = (absY || 0) - (textSelection?.startPointY || 0);
  return toSelIndex(ctx?.textSelection?.index, selX, selY);
};

const selIndexInRange = (idx: SelIndexType, start: SelIndexType, end: SelIndexType): boolean => {
  return (idx >= start && idx <= end) || (idx >= end && idx <= start);
};

export type KonvaTextSelectionType = {
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
  addObjectToSelection: (selIndex: SelIndexType, object: SelectionObject) => void;
}

const TextSelectionContext = createContext<TextSelectionContextValue | null>(null);

export interface KonvaTextSelectionProviderProps {
  children: React.ReactNode;
}

export const KonvaTextSelectionProvider = ({ children }: KonvaTextSelectionProviderProps) => {
  const mapOfTokens = useRef<Map<SelIndexType, string>>(new Map());
  const mapOfObjects = useRef<Map<SelIndexType, SelectionObject>>(new Map());
  const [selectedText, setSelectedText] = useState<string>('');
  const [selectedObjects, setSelectedObjects] = useState<SelectionObject[]>([]);
  const [selectedArea, setSelectedArea] = useState<string>('default');
  const [firstSelectedIdx, setFirstSelectedIdx] = useState<SelIndexType | undefined>();
  const [lastSelectedIdx, setLastSelectedIdx] = useState<SelIndexType | undefined>();
  const [selecting, setSelecting] = useState(false);
  const [cleared, setCleared] = useState(false);

  useEffect(() => {
    if (selecting) setCleared(false);
  }, [selecting, setCleared]);

  useEffect(() => {
    if (firstSelectedIdx === undefined || lastSelectedIdx === undefined) return;
    setTimeout(() => {
      extractSelectedText();
      extractSelectedObjects();
    }, 0);
  }, [firstSelectedIdx, lastSelectedIdx]);

  const selectedAreaType = useMemo(() => selectedArea.split('-').reverse()[0], [selectedArea]);

  const selectionMapToList = (selection: Map<SelIndexType, unknown>) => {
    if (firstSelectedIdx === undefined || lastSelectedIdx === undefined) return [];
    const objects = Array.from(selection.entries())
      .filter(([idx]) => selIndexInRange(idx, firstSelectedIdx, lastSelectedIdx)) // TODO: you can avoid filter (see StaticTextBlock)
      .sort(([idx1], [idx2]) => (idx1 > idx2 ? 1 : -1))
      .map(([, object]) => object);
    return objects;
  };

  const extractSelectedText = useCallback(() => {
    const tokens = selectionMapToList(mapOfTokens.current);
    const text = tokens.join('');
    setSelectedText(text);
  }, [selectionMapToList, mapOfTokens]);

  const extractSelectedObjects = useCallback(() => {
    const objects = selectionMapToList(mapOfObjects.current);
    const uniqueObjects = Array.from(new Set(objects));
    setSelectedObjects(uniqueObjects);
  }, [selectionMapToList, mapOfObjects]);

  const clearSelection = useCallback(() => {
    if (cleared) return;
    mapOfTokens.current.clear();
    mapOfObjects.current.clear();
    setFirstSelectedIdx(undefined);
    setLastSelectedIdx(undefined);
    setSelecting(false);
    setSelectedArea('default');
    setSelectedText('');
    setSelectedObjects([]);
    setCleared(true);
  }, [
    setFirstSelectedIdx,
    setLastSelectedIdx,
    setSelecting,
    setSelectedText,
    setSelectedObjects,
    cleared,
    setCleared,
  ]);

  const stopSelecting = useCallback(() => {
    if (selecting) {
      setSelecting(false);
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
    (selIndex: SelIndexType, object: SelectionObject) => {
      mapOfObjects.current.set(selIndex, object);
    },
    [mapOfObjects],
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

export function useKonvaTextSelection() {
  const context = useContext(TextSelectionContext);
  if (!context) {
    throw new Error('useKonvaTextSelection must be used within a KonvaTextSelectionProvider');
  }
  return context;
}

export const KonvaTextSelectionBridgeProvider = TextSelectionContext.Provider;
export const KonvaTextSelectionBridgeConsumer = TextSelectionContext.Consumer;
