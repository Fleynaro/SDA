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
import { withCrash_ } from 'providers/CrashProvider';
import { Line, LineColumn, TokenizedTextView } from 'components/TokenizedTextView';
import { useHighlightedGroupIndexes } from './helpers';
import { Popper, usePopper } from 'components/Popper';
import { ConstantValuePopper } from '../Poppers/ConstantValuePopper';
import { IRcodeVariablePopper } from '../Poppers/IRcodeVariablePopper';
import { SemanticsObject } from 'sda-electron/api/researcher';

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
  const [selectedTokens, setSelectedTokens] = useState<Token[]>([]);
  const highlightedGroupIdxs = useHighlightedGroupIndexes(text);
  const popper = usePopper();

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

  const onClickHighlightBySemantics = useCallback(
    (object: SemanticsObject) => {
      if (!text) return;
      const selectedGroupIdx = new Set<number>();
      for (const { idx, action } of text.groups) {
        if (action.name === IRcodeTokenGroupAction.Value) {
          const { value } = action as IRcodeValueTokenGroupAction;
          if (value.type === 'variable') {
            if (
              object.variables.find(
                (variable) =>
                  variable.functionId.programId.key === value.id.functionId.programId.key &&
                  variable.functionId.offset === value.id.functionId.offset &&
                  variable.variableId === value.id.variableId,
              )
            ) {
              selectedGroupIdx.add(idx);
            }
          }
        }
      }
      setSelectedTokens(text.tokens.filter((token) => selectedGroupIdx.has(token.groupIdx)));
    },
    [text, setSelectedTokens],
  );

  const onClosePopper = useCallback(() => {
    popper.close();
    if (selectedTokens.length === 1) {
      setSelectedTokens([]);
    }
  }, [popper.close, selectedTokens, setSelectedTokens]);

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
            setSelectedTokens([token]);
          }, 500);
        }
      } else if (action.name === IRcodeTokenGroupAction.Value) {
        const { value } = action as IRcodeValueTokenGroupAction;
        if (value.type === 'variable') {
          popper.withTimer(async () => {
            popper.openAtPos(e.clientX, e.clientY + 10);
            popper.setContent(
              <IRcodeVariablePopper
                variableId={value.id}
                onClickHighlightBySemantics={onClickHighlightBySemantics}
              />,
            );
            setSelectedTokens([token]);
          }, 500);
        }
      }
    },
    [text, popper, setSelectedTokens, onClickHighlightBySemantics],
  );

  const onTokenMouseLeave = useCallback(() => {
    popper.withTimer(() => {
      onClosePopper();
    }, 500);
  }, [popper, onClosePopper]);

  if (!text) return null;
  return (
    <>
      <TokenizedTextView
        name={`${image.id.key}-ircode-view`}
        text={text}
        tokenTypeToColor={TokenTypeToColor}
        highlightedGroupIdxs={highlightedGroupIdxs}
        highlightedTokens={selectedTokens}
        columns={splitIntoColumns ? columns : undefined}
        onTokenMouseEnter={onTokenMouseEnter}
        onTokenMouseLeave={onTokenMouseLeave}
      />
      <Popper {...popper.props} onClose={onClosePopper} closeOnMouseLeave />
    </>
  );
};
