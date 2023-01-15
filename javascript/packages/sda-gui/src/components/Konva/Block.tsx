import { Group, KonvaNodeEvents, Rect } from 'react-konva';
import {
  createContext,
  forwardRef,
  MutableRefObject,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
  useImperativeHandle,
} from 'react';
import { TextStyle, TextStyleType, useTextStyle } from './TextStyle';

interface BlockChildInfo {
  width: number | string;
  height: number | string;
  margin: {
    left: number;
    right: number;
    top: number;
    bottom: number;
  };
  empty?: boolean;
}

const mkEmptyBlockChildInfo = (): BlockChildInfo => ({
  width: 0,
  height: 0,
  margin: {
    left: 0,
    right: 0,
    top: 0,
    bottom: 0,
  },
  empty: true,
});

interface BlockChildContextValue {
  childIdxCounter: MutableRefObject<number>;
  forceUpdateCounter: number;
  hasChilds: boolean;
  getParentSize: () => { width: number; height: number };
  getPos: (idx: number) => { x: number; y: number };
  getSize: (idx: number) => { width: number; height: number };
  updateChild: (idx: number, info: BlockChildInfo) => void;
  calculateIndexes: () => void;
}

const BlockChildContext = createContext<BlockChildContextValue | null>(null);

type BlockProps = KonvaNodeEvents & {
  idx?: number;
  x?: number;
  y?: number;
  width?: number | string;
  height?: number | string;
  padding?: {
    left?: number;
    right?: number;
    top?: number;
    bottom?: number;
  };
  margin?: {
    left?: number;
    right?: number;
    top?: number;
    bottom?: number;
  };
  flexDir?: 'row' | 'col';
  fill?: string;
  textStyle?: TextStyleType;
  children?: React.ReactNode;
  onUpdate?: () => void;
};

export interface BlockRef {
  getParentSize: () => { width: number; height: number };
  getPos: (idx: number) => { x: number; y: number };
  getSize: (idx: number) => { width: number; height: number };
}

const parseSize = (size?: number | string, parentSize?: number, freeSize?: number) => {
  if (typeof size === 'number') {
    return size;
  }
  if (size === 'grow' && freeSize && freeSize > 0) {
    return freeSize;
  }
  if (size && parentSize && size.endsWith('%')) {
    const percent = parseFloat(size);
    return parentSize * (percent / 100);
  }
  return 0;
};

export const Block = forwardRef((props: BlockProps, ref: React.Ref<BlockRef>) => {
  const ctx = useContext(BlockChildContext);
  const ctxTextStyle = useTextStyle();
  const [childs, setChilds] = useState<BlockChildInfo[]>([]);

  useEffect(() => {
    props.onUpdate?.();
  }, [childs]);

  const [forceUpdateCounter, setForceUpdateCounter] = useState(0);
  const childIdxCounter = useRef(0);
  const idx = useMemo(() => {
    if (!ctx) return 0;
    if (props.idx !== undefined) return props.idx;
    return ctx.childIdxCounter.current++;
  }, [props.idx, ctx?.forceUpdateCounter]);

  //const idx = props.idx;
  const flexDir = props.flexDir || 'row';
  const realX = props.x || ctx?.getPos(idx).x || 0;
  const realY = props.y || ctx?.getPos(idx).y || 0;
  const { width, height } = ctx?.getSize(idx) || {
    width: parseSize(props.width),
    height: parseSize(props.height),
  };
  const paddingLeft = props.padding?.left || 0;
  const paddingRight = props.padding?.right || 0;
  const paddingTop = props.padding?.top || 0;
  const paddingBottom = props.padding?.bottom || 0;
  const marginLeft = props?.margin?.left || 0;
  const marginRight = props?.margin?.right || 0;
  const marginTop = props?.margin?.top || 0;
  const marginBottom = props?.margin?.bottom || 0;
  const textStyle = { ...ctxTextStyle, ...props.textStyle };

  if (props.fill) {
    if (props.idx === 0) {
      console.log('name', props.fill, 'idx', idx, 'childs', childs, 'realX', realX, 'realY', realY);
      if (childs.length === 0) {
        console.log('no childs');
      }
    }
  }

  const [freeWidth, freeHeight] = useMemo(() => {
    let freeWidth = width - paddingLeft - paddingRight;
    let freeHeight = height - paddingTop - paddingBottom;
    for (let i = 0; i < childs.length; i++) {
      const margin = childs[i].margin;
      freeWidth -= parseSize(childs[i].width, width) + margin.left + margin.right;
      freeHeight -= parseSize(childs[i].height, height) + margin.top + margin.bottom;
    }
    return [freeWidth, freeHeight];
  }, [childs, width, height, paddingLeft, paddingRight, paddingTop, paddingBottom]);

  const getSize = useCallback(
    (idx: number) => {
      if (!childs[idx]) {
        return { width: 0, height: 0 };
      }
      const { width: childWidth, height: childHeight } = childs[idx];
      return {
        width: parseSize(childWidth, width, freeWidth),
        height: parseSize(childHeight, height, freeHeight),
      };
    },
    [childs, width, height, freeWidth, freeHeight],
  );

  const [childAggregation, realWidth, realHeight] = useMemo(() => {
    const agg: { x: number; y: number }[] = [];
    let maxWidth = paddingLeft;
    let maxHeight = paddingTop;
    for (let i = 0; i < childs.length; i++) {
      const childSize = getSize(i);
      const childMargin = childs[i].margin;
      if (i > 0) {
        const prevAgg = agg[i - 1];
        const prevChildSize = getSize(i - 1);
        if (flexDir === 'col') {
          agg.push({
            x: prevAgg.x + childMargin.left,
            y: prevAgg.y + prevChildSize.height + childs[i - 1].margin.bottom + childMargin.top,
          });
        } else {
          const newX =
            prevAgg.x + prevChildSize.width + childs[i - 1].margin.right + childMargin.left;
          if (!width || newX + childMargin.right + childSize.width <= width - paddingRight) {
            agg.push({ x: newX, y: prevAgg.y + childMargin.top });
          } else {
            agg.push({ x: paddingLeft + childMargin.left, y: maxHeight });
          }
        }
      } else {
        agg.push({ x: paddingLeft + childMargin.left, y: paddingTop + childMargin.top });
      }
      maxWidth = Math.max(maxWidth, agg[i].x + childSize.width);
      maxHeight = Math.max(maxHeight, agg[i].y + childSize.height);
    }
    maxWidth += paddingRight;
    maxHeight += paddingBottom;
    return [agg, maxWidth, maxHeight];
  }, [childs, flexDir, width, height, paddingLeft, paddingRight, paddingTop, paddingBottom]);

  useEffect(() => {
    if (!ctx) return;
    if (props.idx !== undefined) return;
    ctx.calculateIndexes();
    return () => {
      ctx.calculateIndexes();
    };
  }, []);

  useEffect(() => {
    if (!ctx) return;
    return () => {
      ctx.updateChild(idx, mkEmptyBlockChildInfo());
    };
  }, [idx]);

  useEffect(() => {
    if (!ctx) return;
    ctx.updateChild(idx, {
      width: props.width || realWidth,
      height: props.height || realHeight,
      margin: {
        left: marginLeft,
        right: marginRight,
        top: marginTop,
        bottom: marginBottom,
      },
    });
  }, [
    idx,
    props.width,
    props.height,
    realWidth,
    realHeight,
    marginLeft,
    marginRight,
    marginTop,
    marginBottom,
  ]);

  const getPos = useCallback(
    (idx: number) => {
      return childAggregation[idx] || { x: 0, y: 0 };
    },
    [childAggregation],
  );

  const updateChild = useCallback(
    (idx: number, info: BlockChildInfo) => {
      setChilds((prev) => {
        const newChilds = [...prev];
        for (let i = 0; i <= idx - prev.length; i++) {
          newChilds.push(mkEmptyBlockChildInfo());
        }
        newChilds[idx] = info;
        for (let i = newChilds.length - 1; i >= 0; i--) {
          if (newChilds[i].empty) {
            newChilds.pop();
          } else {
            break;
          }
        }
        return newChilds;
      });
    },
    [setChilds],
  );

  const calculateIndexes = useCallback(() => {
    childIdxCounter.current = 0;
    setForceUpdateCounter((prev) => prev + 1);
  }, [setForceUpdateCounter]);

  const finalWidth = width || realWidth;
  const finalHeight = height || realHeight;

  const getParentSize = useCallback(() => {
    return { width: finalWidth, height: finalHeight };
  }, [finalWidth, finalHeight]);

  useImperativeHandle(ref, () => ({
    getParentSize,
    getPos,
    getSize,
  }));

  // if (ctx && !ctx.hasChilds) {
  //   return <></>;
  // }

  return (
    <BlockChildContext.Provider
      value={{
        childIdxCounter,
        forceUpdateCounter,
        hasChilds: childs.length > 0,
        getParentSize,
        getPos,
        getSize,
        updateChild,
        calculateIndexes,
      }}
    >
      <TextStyle {...textStyle}>
        <Group {...props} x={realX} y={realY} width={finalWidth} height={finalHeight}>
          {props.fill && <Rect width={finalWidth} height={finalHeight} fill={props.fill} />}

          {props.children}
        </Group>
      </TextStyle>
    </BlockChildContext.Provider>
  );
});
