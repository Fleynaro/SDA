import { useEffect, useMemo, useState } from 'react';
import { TokenGroup, TokenizedText } from 'sda-electron/api/common';
import { Image } from 'sda-electron/api/image';
import {
  PcodeFunctionGraph,
  PcodeInstructionTokenGroupAction,
  PcodeTokenGroupAction,
  getPcodeApi,
} from 'sda-electron/api/p-code';
import { withCrash_ } from 'providers/CrashProvider';
import { TokenizedTextView } from 'components/TokenizedTextView';
import { useSelectedObjects } from 'components/Text';

const TokenTypeToColor = {
  ['Mneumonic']: '#eddaa4',
  ['Register']: '#93c5db',
  ['VirtRegister']: '#93c5db',
  ['Number']: '#e8e8e8',
  ['Keyword']: '#4287f5',
  ['Comment']: 'green',
} as { [type: string]: string };

export interface PcodeViewProps {
  image: Image;
  funcGraph: PcodeFunctionGraph;
}

export const PcodeView = ({ image, funcGraph }: PcodeViewProps) => {
  const selectedObjects = useSelectedObjects();
  const [text, setText] = useState<TokenizedText | null>(null);

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

  const highlightedGroupIdxs = useMemo(() => {
    if (!text) return [];
    const result: number[] = [];
    for (const selObject of selectedObjects) {
      const selGroup = selObject as TokenGroup;
      if (selGroup.action.name !== PcodeTokenGroupAction.Instruction) continue;
      const { offset: selOffset } = selGroup.action as PcodeInstructionTokenGroupAction;
      for (const group of text.groups) {
        if (group.action.name !== PcodeTokenGroupAction.Instruction) continue;
        const { offset } = group.action as PcodeInstructionTokenGroupAction;
        if (offset === selOffset) {
          result.push(group.idx);
        }
      }
    }
    return result;
  }, [selectedObjects, text]);

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
