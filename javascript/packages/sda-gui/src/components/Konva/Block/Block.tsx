import React from 'react';
import { RenderBlock, RenderBlockProps } from './RenderBlock';
import { TextStyleType } from './StaticTextBlock';
import { KonvaTextSelectionType } from './KonvaTextSelection';

export type BlockProps = {
  key?: string | number;
  x?: number;
  y?: number;
  width?: number | string;
  height?: number | string;
  parentWidth?: number;
  parentHeight?: number;
  freeWidth?: number;
  freeHeight?: number;
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
  inline?: boolean;
  fill?: string;
  textStyle?: TextStyleType;
  textSelection?: KonvaTextSelectionType;
  setStartSelectionPointHere?: boolean;
  ctx?: any;
  render?: React.ReactNode;
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
  const x = props.x || 0;
  const y = props.y || 0;
  const width = parseSize(props.width, props.parentWidth, props.freeWidth);
  const height = parseSize(props.height, props.parentHeight, props.freeHeight);
  const padding = {
    left: props.padding?.left || 0,
    right: props.padding?.right || 0,
    top: props.padding?.top || 0,
    bottom: props.padding?.bottom || 0,
  };
  const margin = {
    left: props?.margin?.left || 0,
    right: props?.margin?.right || 0,
    top: props?.margin?.top || 0,
    bottom: props?.margin?.bottom || 0,
  };
  const flexDir = props.flexDir || 'row';
  const ctx = {
    textStyle: {
      ...props.ctx?.textStyle,
      ...props.textStyle,
    },
    textSelection: {
      ...props.ctx?.textSelection,
      ...props.textSelection,
      area:
        (props.ctx?.textSelection?.area || '') +
        (props.textSelection?.area ? `-${props.textSelection.area}` : ''),
    },
  };

  if (
    (props.width === 'grow' && !props.freeWidth) ||
    (props.height === 'grow' && !props.freeHeight)
  ) {
    const build = (newProps: BlockProps) => {
      return Block({
        ...props,
        ...newProps,
      });
    };
    // return empty block to be filled later
    return (
      <RenderBlock
        x={x}
        y={y}
        absX={0}
        absY={0}
        width={0}
        height={0}
        margin={margin}
        build={build}
      />
    );
  }

  const toRenderBlock = (e: JSX.Element): JSX.Element | null => {
    if (e?.type?.name === 'RenderBlock') return e;
    if (typeof e.type === 'function') {
      let eprops = {
        ...e.props,
        ctx,
      };
      if (e.type.name === 'Block') {
        eprops = {
          ...eprops,
          parentWidth: width,
          parentHeight: height,
        };
      }
      const derivative = e.type(eprops) as JSX.Element;
      return toRenderBlock(derivative);
    }
    return null;
  };

  const childs = React.Children.toArray(props.children) as JSX.Element[];
  const childRenderBlocks = childs
    .map(toRenderBlock)
    .filter((child) => child !== null) as JSX.Element[];
  for (let i = 0; i < childRenderBlocks.length; i++) {
    const child: RenderBlockProps = childRenderBlocks[i].props;
    if (child.inline) {
      const childsOfChild = React.Children.toArray(child.children) as JSX.Element[];
      childRenderBlocks.splice(i, 1, ...childsOfChild);
      i--;
    }
  }

  const calcFreeSpace = () => {
    let freeWidth = width - padding.left - padding.right;
    let freeHeight = height - padding.top - padding.bottom;
    for (let i = 0; i < childRenderBlocks.length; i++) {
      const child: RenderBlockProps = childRenderBlocks[i].props;
      if (!child.build) {
        freeWidth -= child.width + child.margin.left + child.margin.right;
        freeHeight -= child.height + child.margin.top + child.margin.bottom;
      }
    }
    return { freeWidth, freeHeight };
  };

  const aggregate = () => {
    const agg: { x: number; y: number }[] = [];
    let maxWidth = padding.left;
    let maxHeight = padding.top;
    for (let i = 0; i < childRenderBlocks.length; i++) {
      const child: RenderBlockProps = childRenderBlocks[i].props;
      if (i > 0) {
        const prevAgg = agg[i - 1];
        const prevChild: RenderBlockProps = childRenderBlocks[i - 1].props;
        if (flexDir === 'col') {
          agg.push({
            x: prevAgg.x + child.margin.left,
            y: prevAgg.y + prevChild.height + prevChild.margin.bottom + child.margin.top,
          });
        } else if (flexDir === 'row') {
          const newX = prevAgg.x + prevChild.width + prevChild.margin.right + child.margin.left;
          if (!width || newX + child.margin.right + child.width <= width - padding.right) {
            agg.push({ x: newX, y: prevAgg.y + child.margin.top });
          } else {
            agg.push({ x: padding.left + child.margin.left, y: maxHeight });
          }
        }
      } else {
        agg.push({ x: padding.left + child.margin.left, y: padding.top + child.margin.top });
      }
      maxWidth = Math.max(maxWidth, agg[i].x + child.width);
      maxHeight = Math.max(maxHeight, agg[i].y + child.height);
    }
    maxWidth += padding.right;
    maxHeight += padding.bottom;
    return { childAggregation: agg, realWidth: maxWidth, realHeight: maxHeight };
  };

  // if (props.fill === 'red') {
  //   console.log('red', childRenderBlocks);
  // }

  // deferred building of children to inject free space size in them (needed for 'grow' size)
  const { freeWidth, freeHeight } = calcFreeSpace();
  for (let i = 0; i < childRenderBlocks.length; i++) {
    const child: RenderBlockProps = childRenderBlocks[i].props;
    if (child.build) {
      childRenderBlocks[i] = child.build({
        freeWidth,
        freeHeight,
      });
    }
  }

  // arrange children and calculate real size of block
  const { childAggregation, realWidth, realHeight } = aggregate();
  for (let i = 0; i < childRenderBlocks.length; i++) {
    const childRenderBlock = childRenderBlocks[i];
    childRenderBlocks[i] = React.cloneElement(childRenderBlock, {
      key: childRenderBlock.key || i,
      x: childAggregation[i].x,
      y: childAggregation[i].y,
    });
  }

  const finalWidth = width || realWidth;
  const finalHeight = height || realHeight;

  return (
    <RenderBlock
      key={props.key}
      x={x}
      y={y}
      absX={x}
      absY={y}
      width={finalWidth}
      height={finalHeight}
      margin={margin}
      fill={props.fill}
      inline={props.inline}
      setStartSelectionPointHere={props.setStartSelectionPointHere}
      render={props.render}
    >
      {childRenderBlocks}
    </RenderBlock>
  );
};
