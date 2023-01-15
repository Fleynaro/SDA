import { Block, StaticText, Text } from 'components/Konva';
import { useCallback } from 'react';
import { ImageBaseRow, ImageInstructionRow, ImageRowType } from 'sda-electron/api/image';
import { useImageContent } from './context';
import { useImageContentStyle } from './style';

interface InstructionRowProps {
  row: ImageInstructionRow;
}

const InstructionRow = ({ row }: InstructionRowProps) => {
  const { styles } = useImageContentStyle();
  return (
    <Block idx={1} width={300}>
      {row.tokens.map((token, i) => (
        <StaticText
          key={i}
          idx={i}
          text={token.text}
          fill={styles.row.instructionTokenColors[token.type] || 'white'}
        />
      ))}
    </Block>
  );
};

interface RowProps {
  idx: number;
  row: ImageBaseRow;
}

export const Row = ({ idx, row }: RowProps) => {
  const {
    rowSelection: { selectedRows, firstSelectedRow, setFirstSelectedRow, setLastSelectedRow },
  } = useImageContent();
  const { styles } = useImageContentStyle();
  const isSelected = selectedRows.includes(row.offset);

  const onMouseDown = useCallback(() => {
    setFirstSelectedRow?.(row.offset);
  }, [setFirstSelectedRow, row.offset]);

  const onMouseMove = useCallback(() => {
    if (firstSelectedRow !== undefined) {
      setLastSelectedRow?.(row.offset);
    }
  }, [firstSelectedRow, setFirstSelectedRow, row.offset]);

  const onMouseUp = useCallback(() => {
    setFirstSelectedRow?.(undefined);
    setLastSelectedRow?.(undefined);
  }, [setFirstSelectedRow, setLastSelectedRow]);

  return (
    <Block
      idx={idx}
      width="100%"
      padding={{ left: 5, top: 5, right: 5, bottom: 5 }}
      fill={isSelected ? '#360b0b' : '#00000000'}
      onMouseDown={onMouseDown}
      onMouseMove={onMouseMove}
      onMouseUp={onMouseUp}
    >
      <Block idx={0} width={100} margin={{ left: 100 }}>
        <StaticText idx={0} text={`0x${row.offset.toString(16)} | ${row.offset}`} fill="white" />
      </Block>
      {row.type === ImageRowType.Instruction && <InstructionRow row={row as ImageInstructionRow} />}
    </Block>
  );
};
