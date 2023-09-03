import { useEffect, useState } from 'react';
import { TokenizedText } from 'sda-electron/api/common';
import { Image } from 'sda-electron/api/image';
import { IRcodeFunction, getIRcodeApi } from 'sda-electron/api/ir-code';
import { withCrash_ } from 'providers/CrashProvider';
import { TokenizedTextView } from 'components/TokenizedTextView';
import { useHighlightedGroupIndexes } from './helpers';

const TokenTypeToColor = {
  ['Operation']: '#eddaa4',
  ['Variable']: '#6c83b8',
  ['Register']: '#93c5db',
  ['VirtRegister']: '#93c5db',
  ['Symbol']: '#bfbfbf',
  ['Number']: '#e8e8e8',
  ['Keyword']: '#4287f5',
  ['Comment']: 'green',
} as { [type: string]: string };

export interface IRcodeViewProps {
  image: Image;
  func: IRcodeFunction;
}

export const IRcodeView = ({ image, func }: IRcodeViewProps) => {
  const [text, setText] = useState<TokenizedText | null>(null);
  const highlightedGroupIdxs = useHighlightedGroupIndexes(text);

  useEffect(
    withCrash_(async () => {
      if (!image || !func) return;
      const tokenizedText = await getIRcodeApi().getIRcodeTokenizedText(image.contextId, func.id);
      setText(tokenizedText);
    }),
    [image, func],
  );

  if (!text) return null;
  return (
    <TokenizedTextView
      name={`${image.id.key}-ircode-view`}
      text={text}
      tokenTypeToColor={TokenTypeToColor}
      highlightedGroupIdxs={highlightedGroupIdxs}
    />
  );
};
