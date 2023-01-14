import { createContext, useContext } from 'react';
import { TextConfig } from 'konva/lib/shapes/Text';

export type TextStyleType = Omit<TextConfig, 'text'> & {
  fill?: string;
};
const TextStyleContext = createContext<TextStyleType>({});

type TextStyleProps = TextStyleType & {
  children: React.ReactNode;
};

export const TextStyle = ({ children, ...textStyle }: TextStyleProps) => {
  return <TextStyleContext.Provider value={textStyle}>{children}</TextStyleContext.Provider>;
};

export const useTextStyle = () => {
  return useContext(TextStyleContext);
};
