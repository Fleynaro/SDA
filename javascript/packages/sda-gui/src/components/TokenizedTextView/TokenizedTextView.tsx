import { useMemo } from 'react';
import { Grid } from '@mui/material';
import { Token, TokenizedText } from 'sda-electron/api/common';

interface TokenizedTextViewProps {
  text: TokenizedText;
}

export const TokenizedTextView = ({ text }: TokenizedTextViewProps) => {
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
      <Grid container direction="column">
        {linesOfTokens.map((tokens, i) => (
          <Grid item key={i}>
            {tokens.map((token, j) => (
              <span key={j}>{token.text}</span>
            ))}
          </Grid>
        ))}
      </Grid>
    );
  }, [linesOfTokens]);

  return content;
};
