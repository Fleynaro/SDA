import { useCallback, useEffect, useState } from 'react';
import { Token, TokenizedText } from 'sda-electron/api/common';
import { Image } from 'sda-electron/api/image';
import { PcodeTokenGroupAction, PcodeVarnodeTokenGroupAction } from 'sda-electron/api/p-code';
import {
  IRcodeFunction,
  IRcodeTokenGroupAction,
  IRcodeValueTokenGroupAction,
  getIRcodeApi,
} from 'sda-electron/api/ir-code';
import { getResearcherApi } from 'sda-electron/api/researcher';
import { withCrash_ } from 'providers/CrashProvider';
import { Line, LineColumn, TokenizedTextView } from 'components/TokenizedTextView';
import { useHighlightedGroupIndexes } from './helpers';
import { usePopperFromContext } from 'components/Popper';
import { ConstantValuePopper } from './ConstantValuePopper';
import { StructurePopper } from './StructurePopper';

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
  splitIntoColumns?: boolean;
}

export const IRcodeView = ({ image, func, splitIntoColumns = true }: IRcodeViewProps) => {
  const [text, setText] = useState<TokenizedText | null>(null);
  const [selectedToken, setSelectedToken] = useState<Token | null>(null);
  const highlightedGroupIdxs = useHighlightedGroupIndexes(text);
  const popper = usePopperFromContext();

  useEffect(
    withCrash_(async () => {
      if (!image || !func) return;
      const tokenizedText = await getIRcodeApi().getIRcodeTokenizedText(image.contextId, func.id);
      setText(tokenizedText);
    }),
    [image, func],
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
          // output ir-code variable
          columns.push({ width: 120, tokens: [] });
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
      } else if (action.name === IRcodeTokenGroupAction.Value) {
        const { value } = action as IRcodeValueTokenGroupAction;
        if (value.type === 'variable') {
          popper.withTimer(async () => {
            const link = await getResearcherApi().findStructureByVariableId(value.id);
            if (!link) return;
            popper.openAtPos(e.clientX, e.clientY + 10);
            popper.setContent(<StructurePopper structure={link.structure} link={link} />);
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
      name={`${image.id.key}-ircode-view`}
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
