import { useEffect, useState } from 'react';
import { TokenizedText } from 'sda-electron/api/common';
import { Image } from 'sda-electron/api/image';
import { PcodeFunctionGraph, getPcodeApi } from 'sda-electron/api/p-code';
import { withCrash_ } from 'providers/CrashProvider';
import { TokenizedTextView } from 'components/TokenizedTextView';
import { useHighlightedGroupIndexes } from './helpers';

const TokenTypeToColor = {
  ['Mneumonic']: '#eddaa4',
  ['Register']: '#93c5db',
  ['VirtRegister']: '#93c5db',
  ['Symbol']: '#bfbfbf',
  ['Number']: '#e8e8e8',
  ['Keyword']: '#4287f5',
  ['Comment']: 'green',
} as { [type: string]: string };

export interface PcodeViewProps {
  image: Image;
  funcGraph: PcodeFunctionGraph;
}

export const PcodeView = ({ image, funcGraph }: PcodeViewProps) => {
  const [text, setText] = useState<TokenizedText | null>(null);
  const highlightedGroupIdxs = useHighlightedGroupIndexes(text);

  useEffect(
    withCrash_(async () => {
      if (!image || !funcGraph) return;
      const tokenizedText = await getPcodeApi().getPcodeTokenizedText(
        image.contextId,
        funcGraph.id,
      );
      setText(tokenizedText);
    }),
    [image, funcGraph],
  );

  if (!text) return null;
  return (
    <TokenizedTextView
      name={`${image.id.key}-pcode-view`}
      text={text}
      tokenTypeToColor={TokenTypeToColor}
      highlightedGroupIdxs={highlightedGroupIdxs}
    />
  );
};
