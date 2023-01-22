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
  render?: React.ReactNode;
  children?: React.ReactNode;
  build?: (newProps: BlockProps) => JSX.Element;
};

export const RenderBlock = (props: RenderBlockProps) => {
  if (props.render) {
    return React.cloneElement(
      props.render as React.ReactElement,
      {
        x: props.x,
        y: props.y,
        width: props.width,
        height: props.height,
        absX: props.absX + props.x,
        absY: props.absY + props.y,
      },
      props.children,
    );
  }
  return (
    <Group x={props.x} y={props.y} width={props.width} height={props.height}>
      {props.fill && <Rect width={props.width} height={props.height} fill={props.fill} />}
      {React.Children.map(props.children, (child) => {
        if (React.isValidElement(child)) {
          return React.cloneElement(child as React.ReactElement, {
            absX: props.absX + props.x,
            absY: props.absY + props.y,
          });
        }
        return child;
      })}
    </Group>
  );
};
