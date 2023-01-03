import {
  createContext,
  useEffect,
  useCallback,
  useContext,
  useMemo,
  useState,
  useRef,
} from 'react';
import { TextConfig } from 'konva/lib/shapes/Text';
import Konva from 'konva';
import { Rect, Text } from 'react-konva';
import { setCursor } from 'components/Konva';

interface TokenPos {
  selectionArea: string;
  textIdx: number;
  tokenIdx: number;
}
const tokenPosToNumber = (pos: TokenPos) => pos.textIdx * 10000 + pos.tokenIdx;
const tokenPosInRange = (pos: TokenPos, start: TokenPos, end: TokenPos): boolean => {
  if (start.selectionArea !== end.selectionArea || start.selectionArea !== pos.selectionArea) {
    return false;
  }
  const posNum = tokenPosToNumber(pos);
  const startNum = tokenPosToNumber(start);
  const endNum = tokenPosToNumber(end);
  if (startNum > endNum) {
    return tokenPosInRange(pos, end, start);
  }
  return posNum >= startNum && posNum <= endNum;
};

interface KonvaFormatTextSelectionContextValue {
  selectedTextRef: React.MutableRefObject<string>;
  firstSelectedTokenPos?: TokenPos;
  setFirstSelectedTokenPos: (pos?: TokenPos) => void;
  lastSelectedTokenPos?: TokenPos;
  setLastSelectedTokenPos: (pos?: TokenPos) => void;
  selecting?: boolean;
  setSelecting: (selecting: boolean) => void;
  clearSelection: () => void;
  stopSelecting: () => void;
}

const KonvaFormatTextSelectionContext = createContext<KonvaFormatTextSelectionContextValue | null>(
  null,
);

export interface KonvaFormatTextSelectionProviderProps {
  children: React.ReactNode;
}

export const KonvaFormatTextSelectionProvider = ({
  children,
}: KonvaFormatTextSelectionProviderProps) => {
  const selectedTextRef = useRef<string>('');
  const [firstSelectedTokenPos, setFirstSelectedTokenPos] = useState<TokenPos | undefined>();
  const [lastSelectedTokenPos, setLastSelectedTokenPos] = useState<TokenPos | undefined>();
  const [selecting, setSelecting] = useState(false);

  const clearSelection = useCallback(() => {
    setFirstSelectedTokenPos(undefined);
    setLastSelectedTokenPos(undefined);
    setSelecting(false);
    selectedTextRef.current = '';
  }, [setFirstSelectedTokenPos, setLastSelectedTokenPos, setSelecting]);

  const stopSelecting = useCallback(() => {
    if (selecting) {
      setSelecting(false);
    } else {
      clearSelection();
    }
  }, [selecting, setSelecting, clearSelection]);

  return (
    <KonvaFormatTextSelectionContext.Provider
      value={{
        selectedTextRef,
        firstSelectedTokenPos,
        setFirstSelectedTokenPos,
        lastSelectedTokenPos,
        setLastSelectedTokenPos,
        selecting,
        setSelecting,
        clearSelection,
        stopSelecting,
      }}
    >
      {children}
    </KonvaFormatTextSelectionContext.Provider>
  );
};

export function useKonvaFormatTextSelection() {
  const context = useContext(KonvaFormatTextSelectionContext);
  if (!context) {
    throw new Error('useKonvaFormatTextSelection must be used within a KonvaFormatText');
  }
  return context;
}

type TokenProps = {
  tokenPos: TokenPos;
  text: string;
} & TextConfig;

const Token = ({ tokenPos, text, ...props }: TokenProps) => {
  const {
    selectedTextRef,
    firstSelectedTokenPos,
    setFirstSelectedTokenPos,
    lastSelectedTokenPos,
    setLastSelectedTokenPos,
    selecting,
    setSelecting,
  } = useKonvaFormatTextSelection();

  const isSelected =
    firstSelectedTokenPos &&
    lastSelectedTokenPos &&
    tokenPosInRange(tokenPos, firstSelectedTokenPos, lastSelectedTokenPos);

  useEffect(() => {
    if (isSelected) {
      const isReversed =
        tokenPosToNumber(firstSelectedTokenPos) > tokenPosToNumber(lastSelectedTokenPos);
      if ((isReversed ? lastSelectedTokenPos : firstSelectedTokenPos) === tokenPos) {
        selectedTextRef.current = '';
      }
      selectedTextRef.current += text;
    }
  }, [firstSelectedTokenPos, lastSelectedTokenPos]);

  const tokenSize = useMemo(() => {
    const konvaText = new Konva.Text(props);
    return konvaText.measureSize(text);
  }, [props, text]);

  const onMouseDown = useCallback(() => {
    setFirstSelectedTokenPos?.(tokenPos);
    setLastSelectedTokenPos?.(undefined);
    setSelecting?.(true);
  }, [setFirstSelectedTokenPos, setSelecting]);

  const onMouseMove = useCallback(() => {
    if (selecting) {
      setLastSelectedTokenPos?.(tokenPos);
    }
  }, [selecting, setLastSelectedTokenPos]);

  const onMouseEnter = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
    setCursor(e, 'text');
  }, []);

  const onMouseLeave = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
    setCursor(e, 'default');
  }, []);

  return (
    <>
      <Rect
        x={props.x}
        y={props.y}
        width={tokenSize.width}
        height={tokenSize.height}
        fill={isSelected ? 'green' : undefined}
      ></Rect>
      <Text
        text={text}
        {...props}
        onMouseDown={onMouseDown}
        onMouseMove={onMouseMove}
        onMouseEnter={onMouseEnter}
        onMouseLeave={onMouseLeave}
      />
    </>
  );
};

interface TextPart {
  text: string;
  style?: TextConfig & { fill: string };
}

export interface KonvaFormatTextProps {
  selectionArea?: string;
  textIdx: number;
  textParts: TextPart[];
  maxWidth?: number;
  newLineInEnd?: boolean;
}

export const buildKonvaFormatText = ({
  selectionArea,
  textIdx,
  textParts,
  maxWidth,
  newLineInEnd,
}: KonvaFormatTextProps) => {
  let result = <></>;
  let x = 0;
  let y = 0;
  let i = 0;
  let lastHeight = 0;
  if (newLineInEnd) {
    const lastStyle = textParts[textParts.length - 1].style;
    textParts = [...textParts, { text: '\n', style: lastStyle }];
  }
  for (const { text, style } of textParts) {
    const tokens = text.split(' ');
    const tokensWithSpace = tokens.map((token, i) =>
      i === tokens.length - 1 ? token : `${token} `,
    );
    for (const token of tokensWithSpace) {
      const konvaText = new Konva.Text(style);
      const textSize = konvaText.measureSize(token);
      const spaceSize = konvaText.measureSize(' ');
      if (maxWidth && x + textSize.width > maxWidth + spaceSize.width) {
        x = 0;
        y += textSize.height;
      }
      result = (
        <>
          {result}
          <Token
            key={i}
            text={token}
            tokenPos={{ selectionArea: selectionArea || 'default', textIdx, tokenIdx: i }}
            x={x}
            y={y}
            {...style}
          />
        </>
      );
      x += textSize.width;
      i++;
      lastHeight = Math.max(lastHeight, textSize.height);
    }
  }
  return {
    height: y + lastHeight,
    elem: result,
  };
};
