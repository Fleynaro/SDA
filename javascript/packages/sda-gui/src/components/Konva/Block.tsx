import { Group, KonvaNodeEvents, Rect } from 'react-konva';
import { createContext, useCallback, useContext, useEffect, useMemo, useState } from 'react';
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
  removed?: boolean;
}

const mkEmptyBlockChildInfo = (removed?: boolean): BlockChildInfo => ({
  width: 0,
  height: 0,
  margin: {
    left: 0,
    right: 0,
    top: 0,
    bottom: 0,
  },
  removed,
});

interface BlockChildContextValue {
  parentWidth: number;
  parentHeight: number;
  getPos: (idx: number) => { x: number; y: number };
  getSize: (idx: number) => { width: number; height: number };
  registerChild: (idx: number, info: BlockChildInfo) => void;
  unregisterChild: (idx: number) => void;
}

const BlockChildContext = createContext<BlockChildContextValue | null>(null);

type BlockProps = KonvaNodeEvents & {
  idx: number;
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
};

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

export const Block = (props: BlockProps) => {
  const ctx = useContext(BlockChildContext);
  const ctxTextStyle = useTextStyle();
  const [childs, setChilds] = useState<BlockChildInfo[]>([]);

  const idx = props.idx;
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

  // console.log('name', props.fill, 'idx', idx);
  useEffect(() => {
    console.log('name', props.fill, 'idx', idx);
  }, []);

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
    ctx.registerChild(idx, {
      width: props.width || realWidth,
      height: props.height || realHeight,
      margin: {
        left: marginLeft,
        right: marginRight,
        top: marginTop,
        bottom: marginBottom,
      },
    });

    return () => {
      ctx.unregisterChild(idx);
    };
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

  const registerChild = useCallback(
    (idx: number, info: BlockChildInfo) => {
      setChilds((prev) => {
        const newChilds = [...prev];
        for (let i = 0; i <= idx - prev.length; i++) {
          newChilds.push(mkEmptyBlockChildInfo());
        }
        newChilds[idx] = info;
        return newChilds;
      });
    },
    [setChilds],
  );

  const unregisterChild = useCallback(
    (idx: number) => {
      setChilds((prev) => {
        const newChilds = [...prev];
        if (idx < prev.length) {
          newChilds[idx] = mkEmptyBlockChildInfo(true);
          for (let i = newChilds.length - 1; i >= 0; i--) {
            if (newChilds[i].removed) {
              newChilds.pop();
            } else {
              break;
            }
          }
        }
        return newChilds;
      });
    },
    [setChilds],
  );

  const finalWidth = width || realWidth;
  const finalHeight = height || realHeight;

  return (
    <BlockChildContext.Provider
      value={{
        parentWidth: finalWidth,
        parentHeight: finalHeight,
        getPos,
        getSize,
        registerChild,
        unregisterChild,
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
};
