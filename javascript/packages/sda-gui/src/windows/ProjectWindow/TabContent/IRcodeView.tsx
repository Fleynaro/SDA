import { useEffect, useMemo, useState } from 'react';
import { TokenGroup, TokenizedText } from 'sda-electron/api/common';
import { Image } from 'sda-electron/api/image';
import { PcodeInstructionTokenGroupAction, PcodeTokenGroupAction } from 'sda-electron/api/p-code';
import { IRcodeFunction, IRcodeTokenGroupAction, getIRcodeApi } from 'sda-electron/api/ir-code';
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

export interface IRcodeViewProps {
  image: Image;
  func: IRcodeFunction;
}

export const IRcodeView = ({ image, func }: IRcodeViewProps) => {
  const selectedObjects = useSelectedObjects();
  const [text, setText] = useState<TokenizedText | null>(null);

  useEffect(
    withCrash_(async () => {
      if (!image || !func) return;
      const tokenizedText = await getIRcodeApi().getIRcodeTokenizedText(image.contextId, func.id);
      setText(tokenizedText);
    }),
    [image, func],
  );

  const highlightedGroupIdxs = useMemo(() => {
    if (!text) return [];
    const result: number[] = [];
    for (const selObject of selectedObjects) {
      const selGroup = selObject as TokenGroup;
      if (selGroup.action.name !== PcodeTokenGroupAction.Instruction) continue;
      const { offset: selOffset } = selGroup.action as PcodeInstructionTokenGroupAction;
      for (const group of text.groups) {
        if (group.action.name !== IRcodeTokenGroupAction.Operation) continue;
        //const { offset } = group.action as IRcodeOperationTokenGroupAction;
      }
    }
    return result;
  }, [selectedObjects, text]);

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
