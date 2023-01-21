import { Group } from 'react-konva';
import React from 'react';
import { BlockProps } from './Block';

export type RenderProps = {
  x?: number;
  y?: number;
  width?: number;
  height?: number;
  children?: React.ReactNode;
};

export type RenderBlockProps = {
  x: number;
  y: number;
  width: number;
  height: number;
  margin: {
    left: number;
    right: number;
    top: number;
    bottom: number;
  };
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
      },
      props.children,
    );
  }
  return (
    <Group x={props.x} y={props.y} width={props.width} height={props.height}>
      {props.children}
    </Group>
  );
};
