import { createContext, useCallback, useContext, useEffect, useRef, useState } from 'react';
import rangy from 'rangy';

type SelectionObject = unknown;

type Extractor = (element: HTMLElement) => SelectionObject | null;

interface HtmlTextSelectionContextValue {
  selectedText: string;
  selectedObjects: SelectionObject[];
  addExtractor: (extractor: Extractor) => () => void;
}

const HtmlTextSelectionContext = createContext<HtmlTextSelectionContextValue | null>(null);

export interface HtmlTextSelectionProviderProps {
  children: React.ReactNode;
}

export const HtmlTextSelectionProvider = ({ children }: HtmlTextSelectionProviderProps) => {
  const [selectedText, setSelectedText] = useState<string>('');
  const [selectedObjects, setSelectedObjects] = useState<SelectionObject[]>([]);
  const extractors = useRef<Extractor[]>([]);

  useEffect(() => {
    const listener = () => {
      const selection = rangy.getSelection();
      setSelectedText(selection.toString());
      if (selection.rangeCount === 0) {
        setSelectedObjects([]);
        return;
      }
      const selectedElements = selection
        .getRangeAt(0)
        .getNodes()
        .filter((node) => node.nodeType === Node.ELEMENT_NODE) as HTMLElement[];
      const selectedObjects = selectedElements
        .flatMap((element) => extractors.current.map((extractor) => extractor(element)))
        .filter((object) => object !== null) as SelectionObject[];
      const uniqueSelObjects = Array.from(new Set(selectedObjects));
      setSelectedObjects(uniqueSelObjects);
    };
    document.addEventListener('selectionchange', listener);
    return () => {
      document.removeEventListener('selectionchange', listener);
    };
  }, [extractors, setSelectedObjects]);

  const addExtractor = useCallback((extractor: Extractor) => {
    extractors.current.push(extractor);
    return () => {
      extractors.current = extractors.current.filter((e) => e !== extractor);
    };
  }, []);

  return (
    <HtmlTextSelectionContext.Provider value={{ selectedText, selectedObjects, addExtractor }}>
      {children}
    </HtmlTextSelectionContext.Provider>
  );
};

export function useHtmlTextSelection() {
  const context = useContext(HtmlTextSelectionContext);
  if (!context) {
    throw new Error('useHtmlTextSelection must be used within a HtmlTextSelectionProvider');
  }
  return context;
}

export const HtmlTextSelectionBridgeProvider = HtmlTextSelectionContext.Provider;
export const HtmlTextSelectionBridgeConsumer = HtmlTextSelectionContext.Consumer;
