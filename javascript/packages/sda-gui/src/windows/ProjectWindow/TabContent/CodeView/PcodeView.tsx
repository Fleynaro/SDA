import { useCallback, useEffect, useState } from 'react';
import { Token, TokenizedText } from 'sda-electron/api/common';
import { Image } from 'sda-electron/api/image';
import {
  PcodeFunctionGraph,
  PcodeTokenGroupAction,
  PcodeVarnodeTokenGroupAction,
  getPcodeApi,
} from 'sda-electron/api/p-code';
import { withCrash_ } from 'providers/CrashProvider';
import { Line, LineColumn, TokenizedTextView } from 'components/TokenizedTextView';
import { useHighlightedGroupIndexes } from './helpers';
import { usePopperFromContext } from 'components/Popper';
import { ConstantValuePopper } from '../Poppers/ConstantValuePopper';

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
  splitIntoColumns?: boolean;
}

export const PcodeView = ({ image, funcGraph, splitIntoColumns = true }: PcodeViewProps) => {
  const [text, setText] = useState<TokenizedText | null>(null);
  const [selectedToken, setSelectedToken] = useState<Token | null>(null);
  const highlightedGroupIdxs = useHighlightedGroupIndexes(text);
  const popper = usePopperFromContext();

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

  const columns = useCallback((line: Line) => {
    const columns: LineColumn[] = [];
    for (let i = 0; i < line.tokens.length; i++) {
      const token = line.tokens[i];
      if (columns.length === 0) {
        if (token.text !== ' ') {
          if (i > 0) {
            // tabs
            columns.push({ width: 2 * (i + 1), tokens: [] });
          }
          // output p-code variable
          columns.push({ width: 90, tokens: [] });
        } else {
          continue;
        }
      } else if (token.text === ' = ') {
        columns.push({ tokens: [] });
      }
      columns[columns.length - 1].tokens.push(token);
    }
    return columns;
  }, []);

  const onTokenMouseEnter = useCallback(
    (e: React.MouseEvent<HTMLSpanElement, MouseEvent>, token: Token) => {
      if (!text) return;
      const { action } = text.groups[token.groupIdx];
      if (action.name === PcodeTokenGroupAction.Varnode) {
        const { varnode } = action as PcodeVarnodeTokenGroupAction;
        if (varnode.type === 'constant') {
          popper.withTimer(() => {
            popper.openAtPos(e.clientX, e.clientY + 10);
            popper.setContent(<ConstantValuePopper value={varnode.value} />);
            popper.setCloseCallback(() => {
              setSelectedToken(null);
            });
            setSelectedToken(token);
          }, 500);
        }
      }
    },
    [text, popper, setSelectedToken],
  );

  const onTokenMouseLeave = useCallback(() => {
    popper.withTimer(() => {
      popper.close();
    }, 500);
  }, [popper]);

  if (!text) return null;
  return (
    <TokenizedTextView
      name={`${image.id.key}-pcode-view`}
      text={text}
      tokenTypeToColor={TokenTypeToColor}
      highlightedGroupIdxs={highlightedGroupIdxs}
      highlightedToken={selectedToken}
      columns={splitIntoColumns ? columns : undefined}
      onTokenMouseEnter={onTokenMouseEnter}
      onTokenMouseLeave={onTokenMouseLeave}
    />
  );
};