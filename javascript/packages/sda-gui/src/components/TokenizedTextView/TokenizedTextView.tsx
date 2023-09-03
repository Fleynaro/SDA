import { useCallback, useEffect, useMemo, useState } from 'react';
import { Grid } from '@mui/material';
import { makeStyles } from '@mui/styles';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import KeyboardArrowRightIcon from '@mui/icons-material/KeyboardArrowRight';
import { Token, TokenizedText } from 'sda-electron/api/common';
import { PcodeStructBlockTokenGroupAction, PcodeTokenGroupAction } from 'sda-electron/api/p-code';
import { useHtmlTextSelection } from 'components/Text';

const row = {
  height: 20,
};

const useStyles = makeStyles(() => ({
  root: {
    cursor: 'default',
    userSelect: 'none',
    whiteSpace: 'nowrap', // prevent line breaks
  },
  leftColumn: {
    width: 50,
    color: '#999999',
  },
  lineIndexColumn: {
    textAlign: 'right',
  },
  collapseCodeColumn: {
    width: 20,
    color: '#a8a8a8',
    opacity: 0,
    transition: 'opacity 0.2s ease-in-out',
    '&:hover': {
      opacity: 1,
    },
  },
  collapseButton: {
    cursor: 'pointer',
  },
  collapseEllipsis: {
    cursor: 'pointer',
    color: '#999999',
  },
  row,
  codeRow: {
    ...row,
    userSelect: 'text',
  },
}));

interface TokenizedTextViewProps {
  name: string;
  text: TokenizedText;
  tokenTypeToColor?: { [type: string]: string };
  highlightedGroupIdxs?: number[];
  highlightedToken?: Token | null;
  onTokenMouseEnter?: (e: React.MouseEvent<HTMLSpanElement, MouseEvent>, token: Token) => void;
  onTokenMouseLeave?: (e: React.MouseEvent<HTMLSpanElement, MouseEvent>, token: Token) => void;
}

interface Line {
  index: number;
  tokens: Token[];
  collapse?: {
    id: string;
    endLine: Line;
  };
}

export const TokenizedTextView = ({
  name,
  text,
  tokenTypeToColor = {},
  highlightedGroupIdxs = [],
  highlightedToken,
  onTokenMouseEnter,
  onTokenMouseLeave,
}: TokenizedTextViewProps) => {
  const classes = useStyles();
  const [selectedTokenText, setSelectedTokenText] = useState<string | null>(null);
  const [collapsedLineIds, setCollapsedLineIds] = useState<string[]>([]);
  const { addExtractor } = useHtmlTextSelection();

  useEffect(() => {
    return addExtractor((e) => {
      if (e.dataset.area === name) {
        return text.groups[Number(e.dataset.groupIdx)];
      }
      return null;
    });
  }, [addExtractor, name, text.groups]);

  // all lines that can be shown
  const allLines = useMemo(() => {
    const lines: Line[] = [];
    // create lines
    for (const token of text.tokens) {
      if (lines.length === 0 || token.text === '\n') {
        lines.push({ index: lines.length, tokens: [] });
        if (token.text === '\n') continue;
      }
      lines[lines.length - 1].tokens.push(token);
    }
    // find lines that can be collapsed
    for (const line of lines) {
      const token = line.tokens.find(
        (token) =>
          token.type === 'Keyword' &&
          text.groups[token.groupIdx].action.name === PcodeTokenGroupAction.StructBlock,
      );
      if (token) {
        const endLine = lines.find(
          (l) =>
            l.index > line.index + 1 &&
            l.tokens.find((t) => t.type === 'Symbol' && t.groupIdx === token.groupIdx),
        );
        if (endLine) {
          const { id } = text.groups[token.groupIdx].action as PcodeStructBlockTokenGroupAction;
          line.collapse = { id, endLine };
        }
      }
    }
    return lines;
  }, [text.tokens]);

  // lines that are not hidden (collapsed)
  const lines = useMemo(() => {
    const hiddenIndexes = new Set<number>();
    // hide if line is in collapsed block
    for (const id of collapsedLineIds) {
      const line = allLines.find((l) => l.collapse?.id === id);
      if (line && line.collapse) {
        for (let i = line.index + 1; i < line.collapse.endLine.index; i++) {
          hiddenIndexes.add(i);
        }
      }
    }
    // ...
    return allLines.filter((l) => !hiddenIndexes.has(l.index));
  }, [collapsedLineIds, allLines]);

  const lineHighlightColor = useMemo(() => {
    const lineHighlightColor = new Map<number, string>();
    for (const line of allLines) {
      if (line.tokens.find((t) => highlightedGroupIdxs.includes(t.groupIdx))) {
        lineHighlightColor.set(line.index, '#304559');
      }
    }
    return lineHighlightColor;
  }, [allLines, highlightedGroupIdxs]);

  const onCollapseCode = useCallback(
    (line: Line) => {
      if (!line.collapse) return;
      const { id } = line.collapse;
      if (collapsedLineIds.includes(id)) {
        setCollapsedLineIds(collapsedLineIds.filter((l) => l !== id));
      } else {
        setCollapsedLineIds([...collapsedLineIds, id]);
      }
    },
    [lines],
  );

  const isLineCollapsed = useCallback(
    (line: Line) => (line.collapse ? collapsedLineIds.includes(line.collapse.id) : false),
    [collapsedLineIds],
  );

  const content = useMemo(() => {
    return (
      <Grid
        container
        onMouseDown={() => setSelectedTokenText(null)}
        direction="row"
        wrap="nowrap"
        className={classes.root}
      >
        <Grid item container direction="row" wrap="nowrap" className={classes.leftColumn}>
          <Grid item container direction="column" className={classes.lineIndexColumn}>
            {lines.map((line) => (
              <Grid item key={line.index} className={classes.row}>
                {line.index + 1}
              </Grid>
            ))}
          </Grid>
          <Grid item container direction="column" className={classes.collapseCodeColumn}>
            {lines.map((line) => (
              <Grid item key={line.index} className={classes.row}>
                {line.collapse && (
                  <div onClick={() => onCollapseCode(line)} className={classes.collapseButton}>
                    {isLineCollapsed(line) ? <KeyboardArrowRightIcon /> : <ExpandMoreIcon />}
                  </div>
                )}
              </Grid>
            ))}
          </Grid>
        </Grid>
        <Grid item container direction="column">
          {lines.map((line) => (
            <Grid
              item
              direction="row"
              className={classes.codeRow}
              key={line.index}
              style={{ backgroundColor: lineHighlightColor.get(line.index) }}
            >
              {line.tokens.map((token, j) => (
                <span
                  key={j}
                  onMouseUp={() => setSelectedTokenText(token.text !== ' ' ? token.text : null)}
                  onMouseEnter={(e) => onTokenMouseEnter?.(e, token)}
                  onMouseLeave={(e) => onTokenMouseLeave?.(e, token)}
                  style={{
                    color: tokenTypeToColor[token.type],
                    backgroundColor:
                      highlightedToken === token || selectedTokenText === token.text
                        ? '#304559'
                        : undefined,
                  }}
                  aria-label={token.type}
                  data-group-idx={token.groupIdx}
                  data-area={name}
                >
                  {token.text === ' ' ? '\u00A0' : token.text}
                </span>
              ))}
              {isLineCollapsed(line) && (
                <>
                  {' '}
                  <span className={classes.collapseEllipsis} onClick={() => onCollapseCode(line)}>
                    ...
                  </span>
                </>
              )}
            </Grid>
          ))}
          <Grid item direction="row" style={{ height: 100 }} aria-label="empty-space"></Grid>
        </Grid>
      </Grid>
    );
  }, [
    lines,
    lineHighlightColor,
    highlightedToken,
    selectedTokenText,
    setSelectedTokenText,
    isLineCollapsed,
    onTokenMouseEnter,
    onTokenMouseLeave,
  ]);

  return content;
};
