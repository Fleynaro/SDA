import { useMemo } from 'react';
import { Block, StaticTextBlock, useStage } from 'components/Konva';
import { Layer, Rect } from 'react-konva';
import { Token, TokenizedText } from 'sda-electron/api/common';

interface TokenizedTextViewProps {
  text: TokenizedText;
}

export const TokenizedTextView = ({ text }: TokenizedTextViewProps) => {
  const stage = useStage();
  const linesOfTokens = useMemo(() => {
    const linesOfTokens: Token[][] = [[]];
    for (const token of text.tokens) {
      linesOfTokens[linesOfTokens.length - 1].push(token);
      if (token.text === '\n') {
        linesOfTokens.push([]);
      }
    }
    return linesOfTokens;
  }, [text.tokens]);

  const content = useMemo(() => {
    return (
      <Block flexDir="col" width={stage.size.width}>
        {linesOfTokens.map((tokens, i) => (
          <Block key={i} padding={{ bottom: 1, top: 1 }} width="100%">
            {tokens.map((token, j) => (
              <Block key={j}>
                <StaticTextBlock text={token.text} fill={'white'} />
              </Block>
            ))}
          </Block>
        ))}
      </Block>
    );
  }, [linesOfTokens, stage.size.width]);

  return (
    <>
      <Layer>
        <Rect width={stage.size.width} height={stage.size.height} fill="black" />
      </Layer>
      <Layer>{content}</Layer>
    </>
  );
};
