import { useCallback } from 'react';
import { ImageRowType, ImageBaseRow, ImageInstructionRow } from 'sda-electron/api/image';
import { PcodeNode } from 'sda-electron/api/p-code';
import { Group, Rect } from 'react-konva';
import { Block, buildKonvaFormatText, TextBlock } from 'components/Konva';
import { StylesType, useImageContentStyle } from './style';
import { useImageContent } from './context';
import { useBlock } from 'components/Konva/Block/RenderBlock';

// export interface PcodeElement {
//   height: number;
//   element: JSX.Element;
// }

// Сделать это как реакт компонент через контекст (концепция потока элементов)
// const buildPcode = (node: PcodeNode, style: StylesType): PcodeElement => {
//   if (node.type === 'token') {
//     const tokenText = buildKonvaFormatText({
//       selectionArea: 'pcode',
//       textIdx: node.idx,
//       textParts: [
//         {
//           text: node.text,
//           style: { fontSize: 12, fill: 'white' },
//         },
//       ],
//       maxWidth: style.row.pcode.width,
//     });
//     return {
//       height: tokenText.height,
//       element: tokenText.elem,
//     };
//   } else if (node.type === 'group') {
//     let height = 0;
//     const childrens = node.childs.map((child) => {
//       const childElem = buildPcode(child, style);
//       height += childElem.height;
//       return childElem.element;
//     });
//     return {
//       height,
//       element: <Group>{childrens}</Group>,
//     };
//   }
//   return {
//     height: 0,
//     element: <></>,
//   };
// };

// export interface ImageRowElement {
//   offset: number;
//   height: number;
//   element: JSX.Element;
// }

// const buildInstructionRow = (row: ImageInstructionRow, style: StylesType): ImageRowElement => {
//   const offsetText = buildKonvaFormatText({
//     selectionArea: 'offset',
//     textIdx: row.offset,
//     textParts: [
//       {
//         text: `0x${row.offset.toString(16)}`,
//         style: { fontSize: 12, fill: 'white' },
//       },
//     ],
//     maxWidth: style.row.cols.instruction.width,
//     newLineInEnd: true,
//   });
//   const instructionText = buildKonvaFormatText({
//     selectionArea: 'instruction',
//     textIdx: row.offset,
//     textParts: row.tokens.map((token) => {
//       return {
//         text: token.text,
//         style: {
//           fontSize: 12,
//           fill: style.row.instructionTokenColors[token.type] || 'white',
//         },
//       };
//     }),
//     maxWidth: style.row.cols.instruction.width,
//     newLineInEnd: true,
//   });
//   let pcode: PcodeElement | undefined;
//   if (row.pcode) {
//     //pcode = buildPcode(row.pcode, style);
//   }
//   const height = instructionText.height + (pcode?.height || 0) + style.row.padding * 2;
//   function Elem() {
//     const {
//       rowSelection: { selectedRows, firstSelectedRow, setFirstSelectedRow, setLastSelectedRow },
//     } = useImageContent();
//     const rowWidth = style.row.width;
//     const isSelected = selectedRows.includes(row.offset);

//     const onMouseDown = useCallback(() => {
//       setFirstSelectedRow?.(row.offset);
//     }, [setFirstSelectedRow]);

//     const onMouseMove = useCallback(() => {
//       if (firstSelectedRow !== undefined) {
//         setLastSelectedRow?.(row.offset);
//       }
//     }, [firstSelectedRow, setFirstSelectedRow]);

//     const onMouseUp = useCallback(() => {
//       setFirstSelectedRow?.(undefined);
//       setLastSelectedRow?.(undefined);
//     }, [setFirstSelectedRow, setLastSelectedRow]);

//     return (
//       <Group
//         width={style.row.width}
//         onMouseDown={onMouseDown}
//         onMouseMove={onMouseMove}
//         onMouseUp={onMouseUp}
//       >
//         <Rect width={rowWidth} height={height} fill={isSelected ? '#360b0b' : '#00000000'} />
//         <Group x={style.row.padding} y={style.row.padding}>
//           <Group width={style.row.cols.jump.width}></Group>
//           <Group
//             x={style.row.padding + style.row.cols.jump.width}
//             width={style.row.cols.offset.width}
//           >
//             {offsetText.elem}
//           </Group>
//           <Group
//             x={style.row.padding + style.row.cols.jump.width + style.row.cols.offset.width}
//             width={style.row.cols.instruction.width}
//           >
//             {instructionText.elem}
//           </Group>
//           {pcode?.element && <Group y={instructionText.height}>{pcode.element}</Group>}
//         </Group>
//       </Group>
//     );
//   }
//   return {
//     offset: row.offset,
//     height,
//     element: <Elem />,
//   };
// };

// export const buildRow = (row: ImageBaseRow, styles: StylesType): ImageRowElement => {
//   if (row.type === ImageRowType.Instruction) {
//     return buildInstructionRow(row as ImageInstructionRow, styles);
//   }
//   return {
//     offset: row.offset,
//     height: 0,
//     element: <></>,
//   };
// };

interface InstructionRowProps {
  row: ImageInstructionRow;
  styles: StylesType;
}

const InstructionRow = ({ row, styles }: InstructionRowProps) => {
  return (
    <Block width={300}>
      {row.tokens.map((token, i) => (
        <TextBlock
          key={i}
          text={token.text}
          fill={styles.row.instructionTokenColors[token.type] || 'white'}
        />
      ))}
    </Block>
  );
};

interface RowProps {
  row: ImageBaseRow;
  styles: StylesType;
}

export const Row = ({ row, styles }: RowProps) => {
  const RowRender = () => {
    const { width, height } = useBlock();
    const {
      rowSelection: { selectedRows, firstSelectedRow, setFirstSelectedRow, setLastSelectedRow },
    } = useImageContent();
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
      <Rect
        width={width}
        height={height}
        fill={isSelected ? '#360b0b' : '#00000000'}
        onMouseDown={onMouseDown}
        onMouseMove={onMouseMove}
        onMouseUp={onMouseUp}
      />
    );
  };

  return (
    <Block width="100%" padding={{ left: 5, top: 5, right: 5, bottom: 5 }} render={<RowRender />}>
      <Block width={100} margin={{ left: 100 }}>
        <TextBlock text={`0x${row.offset.toString(16)} | ${row.offset}`} fill="white" />
      </Block>
      {row.type === ImageRowType.Instruction && (
        <InstructionRow row={row as ImageInstructionRow} styles={styles} />
      )}
    </Block>
  );
};
