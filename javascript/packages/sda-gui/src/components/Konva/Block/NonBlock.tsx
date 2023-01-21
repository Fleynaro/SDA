import React from 'react';
import { RenderBlock } from './RenderBlock';

export type NonBlockProps = {
  children?: React.ReactNode;
};

export const NonBlock = (props: NonBlockProps) => {
  return (
    <RenderBlock x={0} y={0} width={0} height={0} margin={{ left: 0, right: 0, top: 0, bottom: 0 }}>
      {props.children}
    </RenderBlock>
  );
};
