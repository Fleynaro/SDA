import { useCallback, useEffect, useState } from 'react';
import { Token, TokenGroup, TokenizedText } from 'sda-electron/api/common';
import { ImageRowType, ImageBaseRow, ImageInstructionRow } from 'sda-electron/api/image';
import { PcodeInstructionTokenGroupAction, PcodeTokenGroupAction } from 'sda-electron/api/p-code';
import { Group, Rect } from 'react-konva';
import {
  Block,
  setCursor,
  StaticTextBlock,
  TextSelectionType,
  toSelIndex,
  toSelIndexFromRenderProps,
  useTextSelection,
} from 'components/Konva';
import { StylesType } from './style';
import { useImageContent } from './context';
import Konva from 'konva';
import { RenderProps } from 'components/Konva';
import { usePopperFromContext } from 'components/Popper';
import { Paper, Button } from '@mui/material';

const PcodePopper = ({ action }: { action: TokenGroup['action'] }) => {
  if (action.name !== PcodeTokenGroupAction.Instruction) {
    return null;
  }
  const { offset } = action as PcodeInstructionTokenGroupAction;
  return (
    <Paper sx={{ p: 5 }}>
      Offset = 0x{offset.toString(16)} <br />
      <Button variant="contained" onClick={() => console.log('action', action)}>
        Some button
      </Button>
    </Paper>
  );
};

interface PcodeTextViewProps {
  pcode: TokenizedText;
  styles: StylesType;
  ctx?: {
    textSelection: TextSelectionType;
  };
}

const PcodeTextView = ({ pcode, styles, ctx }: PcodeTextViewProps) => {
  const linesOfTokens: Token[][] = [[]];
  for (const token of pcode.tokens) {
    linesOfTokens[linesOfTokens.length - 1].push(token);
    if (token.text === '\n') {
      linesOfTokens.push([]);
    }
  }

  const LineRender = (props: RenderProps) => {
    const isDebugging = false; // useDebugging();
    return (
      <Group {...props}>
        {isDebugging && <Rect width={props.width} height={props.height} fill="#db5a5a" />}
        {props.children}
      </Group>
    );
  };

  const TokenRender = (props: RenderProps & { group: TokenGroup }) => {
    const action = props.group.action;
    const popper = usePopperFromContext();
    const { addObjectToSelection, selectionContains } = useTextSelection();
    const selIndex = toSelIndexFromRenderProps(props, ctx);
    const isSelected = selectionContains(selIndex, ctx?.textSelection.area);
    const [selectedByPopper, setSelectedByPopper] = useState(false);

    useEffect(() => {
      if (isSelected) {
        if (action.name === PcodeTokenGroupAction.Instruction) {
          addObjectToSelection(selIndex, props.group, props.group.idx);
        }
      }
    }, [isSelected]);

    const onMouseEnter = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      if (action.name === PcodeTokenGroupAction.Instruction) {
        popper.withTimer(() => {
          popper.openAtPos(e.evt.clientX, e.evt.clientY + 10);
          popper.setContent(<PcodePopper action={action} />);
          popper.setCloseCallback(() => {
            setSelectedByPopper(false);
          });
          setSelectedByPopper(true);
        }, 500);
      }
    }, []);

    const onMouseLeave = useCallback(() => {
      popper.withTimer(() => {
        if (!popper.props.hovered.current) {
          popper.close();
        }
      }, 500);
    }, []);

    return (
      <Group {...props} onMouseEnter={onMouseEnter} onMouseLeave={onMouseLeave}>
        {selectedByPopper && <Rect width={props.width} height={props.height} fill="#eb4d77" />}
        {props.children}
      </Group>
    );
  };

  return (
    <Block flexDir="col">
      {linesOfTokens.map((tokens, i) => (
        <Block key={i} render={<LineRender />}>
          {tokens.map((token, j) => (
            <Block key={j} render={<TokenRender group={pcode.groups[token.groupIdx]} />}>
              <StaticTextBlock
                text={token.text}
                fill={styles.row.pcode.tokenColors[token.type] || 'white'}
              />
            </Block>
          ))}
        </Block>
      ))}
    </Block>
  );
};

interface InstructionRowProps {
  row: ImageInstructionRow;
  styles: StylesType;
}

const InstructionRow = ({ row, styles }: InstructionRowProps) => {
  const tokens = row.tokens.concat({ text: '\n', type: 'Other' });
  return (
    <Block width={styles.row.cols.instruction.width} flexDir="col">
      <Block textSelection={{ area: 'instruction' }}>
        {tokens.map((token, i) => (
          <StaticTextBlock
            key={i}
            text={token.text}
            fill={styles.row.instruction.tokenColors[token.type] || 'white'}
          />
        ))}
      </Block>
      {row.pcode && (
        <Block textSelection={{ area: 'pcode' }} padding={{ left: 10 }}>
          <PcodeTextView pcode={row.pcode} styles={styles} />
        </Block>
      )}
    </Block>
  );
};

interface RowProps {
  rowIdx: number;
  row: ImageBaseRow;
  styles: StylesType;
}

export const Row = ({ rowIdx, row, styles }: RowProps) => {
  const RowRender = ({ absX, absY, x, y, width, height, children }: RenderProps) => {
    const {
      rowSelection: { selectedRows, firstSelectedRow, setFirstSelectedRow, setLastSelectedRow },
    } = useImageContent();
    const { selecting, setLastSelectedIdx } = useTextSelection();
    const isSelected = selectedRows.includes(row.offset);

    const onMouseDown = useCallback(() => {
      setFirstSelectedRow?.(row.offset);
    }, [setFirstSelectedRow, row.offset]);

    const onMouseMove = useCallback(() => {
      if (firstSelectedRow !== undefined) {
        setLastSelectedRow?.(row.offset);
      }
    }, [firstSelectedRow, setLastSelectedIdx, row.offset]);

    const onMouseMoveForBackground = useCallback(
      (e: Konva.KonvaEventObject<MouseEvent>) => {
        if (selecting) {
          const pos = e.target.getStage()?.getPointerPosition();
          if (pos && absX !== undefined && absY !== undefined) {
            setLastSelectedIdx?.(toSelIndex(rowIdx, pos.x - absX, pos.y - absY));
            //console.log('setLastSelectedIdx', rowIdx, pos.x, pos.y);
          }
        }
        // for jump hover event (onMouseLeave not always working there)
        setCursor(e, 'default');
      },
      [absX, absY, selecting, setLastSelectedIdx],
    );

    const onMouseUp = useCallback(() => {
      setFirstSelectedRow?.(undefined);
      setLastSelectedRow?.(undefined);
    }, [setFirstSelectedRow, setLastSelectedRow]);

    return (
      <Group
        x={x}
        y={y}
        width={width}
        height={height}
        onMouseDown={onMouseDown}
        onMouseMove={onMouseMove}
        onMouseUp={onMouseUp}
      >
        <Rect
          width={width}
          height={height}
          fill={isSelected ? '#360b0b' : '#00000000'}
          onMouseMove={onMouseMoveForBackground}
        />
        {children}
        {/* <Text text={`${rowIdx}: ${absX}, ${absY}`} x={700} y={10} fill="green" /> */}
      </Group>
    );
  };

  return (
    <Block
      width="100%"
      padding={{ left: 5, top: 5, right: 5, bottom: 5 }}
      render={<RowRender />}
      setStartSelectionPointHere
    >
      <Block
        width={styles.row.cols.offset.width}
        margin={{ left: styles.row.cols.jump.width }}
        textSelection={{ area: 'offset' }}
      >
        <StaticTextBlock text={`0x${row.offset.toString(16)}| ${row.offset}\n`} fill="white" />
      </Block>
      {row.type === ImageRowType.Instruction && (
        <InstructionRow row={row as ImageInstructionRow} styles={styles} />
      )}
    </Block>
  );
};
