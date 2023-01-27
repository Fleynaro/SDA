import { useCallback, useEffect } from 'react';
import { ImageRowType, ImageBaseRow, ImageInstructionRow } from 'sda-electron/api/image';
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
import { RenderProps } from 'components/Konva';
import Konva from 'konva';
import { PcodeText, PcodeToken } from 'sda-electron/api/p-code';

interface PcodeTextViewProps {
  pcode: PcodeText;
  styles: StylesType;
  ctx?: {
    textSelection: TextSelectionType;
  };
}

const PcodeTextView = ({ pcode, styles, ctx }: PcodeTextViewProps) => {
  const linesOfTokens: PcodeToken[][] = [[]];
  for (const token of pcode.tokens) {
    linesOfTokens[linesOfTokens.length - 1].push(token);
    if (token.text === '\n') {
      linesOfTokens.push([]);
    }
  }
  const TokenRender = (props: RenderProps & { text: string }) => {
    const { mapOfObjects, selectionContains } = useTextSelection();
    const selIndex = toSelIndexFromRenderProps(props, ctx);
    const isSelected = selectionContains(selIndex, ctx?.textSelection.area);
    useEffect(() => {
      if (isSelected) {
        mapOfObjects.current.set(selIndex, props.text);
        console.log('pcode token = ', props.text);
      }
    }, [isSelected]);
    return <Group {...props}>{props.children}</Group>;
  };
  return (
    <Block flexDir="col">
      {linesOfTokens.map((tokens, i) => (
        <Block key={i}>
          {tokens.map((token, j) => (
            <Block key={j} render={<TokenRender text={token.text} />}>
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
        <Block textSelection={{ area: 'p-code' }} padding={{ left: 10 }}>
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
