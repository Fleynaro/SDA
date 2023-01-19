import Konva from 'konva';
import { TextConfig } from 'konva/lib/shapes/Text';
import { Block } from './Block';
import { NonBlock } from './NonBlock';
import { Text } from 'react-konva';

export type TextStyleType = Omit<TextConfig, 'text'> & {
  fill?: string;
};

export type TextBlockProps = TextStyleType & {
  text: string;
};

export const StaticTextBlock = ({ text, ...propsStyle }: TextBlockProps) => {
  const style = { ...propsStyle };
  const konvaText = new Konva.Text(style);
  const textSize = konvaText.measureSize(text);
  return (
    <Block width={textSize.width} height={textSize.height}>
      <NonBlock>
        <Text text={text} {...style} />
      </NonBlock>
    </Block>
  );
};
