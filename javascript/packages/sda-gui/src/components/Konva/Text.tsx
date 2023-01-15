import { useMemo } from 'react';
import { useTextStyle } from './TextStyle';
import { TextProps, StaticText } from './StaticText';

export const Text = ({ idx, text, ...propsStyle }: TextProps) => {
  const ctxStyle = useTextStyle();
  const style = { ...ctxStyle, ...propsStyle };
  const tokens = useMemo(() => {
    return text.split(' ').map((token, i) => (i === text.length - 1 ? token : `${token} `));
  }, [text]);
  return (
    <>
      {tokens.map((token, i) => (
        <StaticText key={i} text={token} {...style} />
      ))}
    </>
  );
};
