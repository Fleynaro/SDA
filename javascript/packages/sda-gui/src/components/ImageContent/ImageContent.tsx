import { useEffect, useMemo, useRef, useState } from 'react';
import { ObjectId } from 'sda-electron/api/common';
import { getImageApi } from 'sda-electron/api/image';
import { useObject } from 'hooks';
import { Group, Layer, Rect, Text } from 'react-konva';
import Konva from 'konva';
import { KonvaStage } from 'components/Konva';

type RowData = {
  offset: number;
  height: number;
  cmd: string;
};

const buildRow = (row: RowData, y: number): [number, JSX.Element] => {
  const rowHeight = row.height * 20;
  const text = new Konva.Text({ fontSize: 10 });
  const textSize = text.measureSize(' text!!! ');
  return [
    rowHeight,
    <Group
      key={row.offset}
      draggable
      y={y}
      ref={(node) => {
        node?.visible(false);
      }}
    >
      <Rect
        width={500}
        height={rowHeight}
        fill="red"
        shadowBlur={5}
        onClick={() => console.log('click')}
      />
      <Text text={row.offset + ': ' + row.cmd} width={500} height={rowHeight} y={5} fill="white" />
    </Group>,
  ];
};

// TODO: сделать новый промежуточный уровень абстракции - древовидный скелет. Далее делаем обрезание невидимых объектов https://codesandbox.io/s/react-konva-simple-windowing-render-10000-lines-2hy2u?file=/src/App.js
// сделать свой scene graph with clipping optimization
const buildRows = (
  rows: RowData[],
  startIndex: number,
  startY: number,
  level: number,
  levelMaxHeights: number[],
): [number, number, JSX.Element] => {
  if (level === 0) {
    const [height, node] = buildRow(rows[startIndex], startY);
    return [startIndex + 1, height, node];
  }
  const result = [];
  let curIndex = startIndex;
  let y = 10;
  while (curIndex < rows.length) {
    const [nextIndex, height, node] = buildRows(rows, curIndex, y, level - 1, levelMaxHeights);
    result.push(node);
    y += height;
    curIndex = nextIndex;
    if (levelMaxHeights[level] && y >= levelMaxHeights[level]) {
      curIndex--;
      break;
    }
  }
  const msg = `lvl = ${level}, startIndex = ${startIndex}, curIndex = ${curIndex}, h = ${y}`;
  return [
    curIndex + 1,
    y,
    <Group key={startIndex} y={startY}>
      <Text text={msg} fill="white" />
      {result}
    </Group>,
  ];
};

export interface ImageContentProps {
  imageId: ObjectId;
}

export function ImageContent({ imageId }: ImageContentProps) {
  const image = useObject(() => getImageApi().getImage(imageId), [imageId]);
  const [rows, setRows] = useState<RowData[]>([]);
  const [scrollY, setScrollY] = useState(0);
  const textRef = useRef<any>(null);
  //const text = new Konva.Text({ fontSize: 10, fontFamily: 'arial' });
  //console.log(text.measureSize(' text!!! '));

  useEffect(() => {
    setRows(
      [...Array(100000)].map((_, i) => ({
        offset: i * 4,
        height: Math.floor(Math.random() * 3) + 1,
        cmd: 'add rax, rbx',
      })),
    );
  }, []);

  const dataNode = useMemo(() => {
    console.time('build');
    const rowsNode = buildRows(rows, 0, 0, 3, [0, 10000, 2000])[2];
    console.timeEnd('build');
    return rowsNode;
  }, [rows]);

  return (
    <>
      <KonvaStage sx={{ width: '100%', height: '100%' }}>
        <Layer y={-scrollY}>{dataNode}</Layer>
        <Layer>
          <Rect
            width={20}
            height={100}
            x={500}
            fill="green"
            draggable
            dragBoundFunc={(pos) => {
              return {
                x: 500,
                y: Math.min(Math.max(pos.y, 0), 500),
              };
            }}
            onDragMove={(e) => {
              setScrollY(e.target.y() * 100);
            }}
          />
          <Text text="hello, guys!" x={500} y={100} fill="white" ref={textRef} />

          <Group
            x={500}
            y={200}
            h={20}
            ref={(node) => {
              node?.visible(false);
            }}
          >
            <Rect width={20} height={100} fill="green" />
            <Rect x={40} width={20} height={50} fill="blue" />
          </Group>
        </Layer>
      </KonvaStage>
    </>
  );
}
