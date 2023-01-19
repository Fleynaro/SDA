import { Group } from 'react-konva';
import React, { createContext, useContext } from 'react';
import { BlockProps } from './Block';

type BlockContextValue = {
  x: number;
  y: number;
  width: number;
  height: number;
};

const BlockContext = createContext<BlockContextValue | null>(null);

export const useBlock = () => {
  const ctx = useContext(BlockContext);
  if (!ctx) throw new Error('BlockContext is not set');
  return ctx;
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
  none?: boolean;
  inline?: boolean;
  children?: React.ReactNode;
  build?: (newProps: BlockProps) => JSX.Element;
};

export const RenderBlock = (props: RenderBlockProps) => {
  return (
    <Group x={props.x} y={props.y} width={props.width} height={props.height}>
      {props.none ? (
        <>{props.children}</>
      ) : (
        <BlockContext.Provider
          value={{
            x: props.x,
            y: props.y,
            width: props.width,
            height: props.height,
          }}
        >
          {props.children}
        </BlockContext.Provider>
      )}
    </Group>
  );
};
