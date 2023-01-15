import Konva from 'konva';
import { Text as KonvaText } from 'react-konva';
import { useMemo } from 'react';
import { Block } from './Block';
import { TextStyleType, useTextStyle } from './TextStyle';

export type TextProps = TextStyleType & {
  idx: number;
  text: string;
};

export const StaticText = ({ idx, text, ...propsStyle }: TextProps) => {
  const ctxStyle = useTextStyle();
  const style = { ...ctxStyle, ...propsStyle };
  const textSize = useMemo(() => {
    const konvaText = new Konva.Text(style);
    return konvaText.measureSize(text);
  }, [text, JSON.stringify(style)]);
  return (
    <Block idx={idx} width={textSize.width} height={textSize.height}>
      <KonvaText text={text} {...style} />
    </Block>
  );
};
