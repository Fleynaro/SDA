import { Group, Rect } from 'react-konva';
import React from 'react';
import { BlockProps } from './Block';

export type RenderProps = {
  x?: number;
  y?: number;
  absX?: number;
  absY?: number;
  width?: number;
  height?: number;
  textSelection?: {
    startPointX?: number;
    startPointY?: number;
  };
  children?: React.ReactNode;
};

export type RenderBlockProps = {
  x: number;
  y: number;
  absX: number;
  absY: number;
  width: number;
  height: number;
  margin: {
    left: number;
    right: number;
    top: number;
    bottom: number;
  };
  fill?: string;
  inline?: boolean;
  textSelection?: {
    startPointX?: number;
    startPointY?: number;
  };
  setStartSelectionPointHere?: boolean;
  render?: React.ReactNode;
  children?: React.ReactNode;
  build?: (newProps: BlockProps) => JSX.Element;
};

export const childrenWithProps = (
  children: React.ReactNode,
  newProps: (childProps: RenderBlockProps) => RenderBlockProps,
) => {
  return React.Children.map(children, (child) => {
    if (React.isValidElement(child)) {
      return React.cloneElement(child as React.ReactElement, newProps(child.props));
    }
    return child;
  });
};

export const RenderBlock = (props: RenderBlockProps) => {
  const absPos = {
    absX: props.absX + props.x,
    absY: props.absY + props.y,
  };
  let childTextSelection = props.textSelection;
  if (props.setStartSelectionPointHere) {
    // set start point to make selection position relative to this render block
    // it is needed because render block can be moved and absolute position changed
    childTextSelection = {
      ...childTextSelection,
      startPointX: absPos.absX,
      startPointY: absPos.absY,
    };
  }
  const childs = childrenWithProps(props.children, (childProps) => ({
    ...childProps,
    ...absPos,
    textSelection: {
      ...childProps.textSelection,
      ...childTextSelection,
    },
  }));
  if (props.render) {
    return React.cloneElement(
      props.render as React.ReactElement,
      {
        x: props.x,
        y: props.y,
        width: props.width,
        height: props.height,
        textSelection: props.textSelection,
        ...absPos,
      } as RenderProps,
      childs,
    );
  }
  return (
    <Group x={props.x} y={props.y} width={props.width} height={props.height}>
      {props.fill && <Rect width={props.width} height={props.height} fill={props.fill} />}
      {childs}
    </Group>
  );
};
