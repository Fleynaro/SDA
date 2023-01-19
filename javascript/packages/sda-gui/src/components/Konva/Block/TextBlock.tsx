import { StaticTextBlock, TextBlockProps } from './StaticTextBlock';
import { Block } from './Block';

export const TextBlock = ({ text, ...propsStyle }: TextBlockProps) => {
  const tokens = text.split(' ').map((token, i) => (i === text.length - 1 ? token : `${token} `));
  return (
    <Block inline>
      {tokens.map((token, i) => (
        <StaticTextBlock key={i} text={token} {...propsStyle} />
      ))}
    </Block>
  );
};
