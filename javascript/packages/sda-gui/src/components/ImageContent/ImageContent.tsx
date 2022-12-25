import { useCallback, useMemo, useRef, useState } from 'react';
import { useEffect } from 'hooks';
import { ObjectId } from 'sda-electron/api/common';
import {
  getImageApi,
  ImageRowType,
  ImageBaseRow,
  ImageInstructionRow,
} from 'sda-electron/api/image';
import { useObject } from 'hooks';
import { Group, Layer, Rect, Text } from 'react-konva';
import Konva from 'konva';
import { useKonvaStage } from 'components/Konva';
import React from 'react';

interface ImageRowElement {
  key: string;
  height: number;
  element: JSX.Element;
}

interface ImageRowElementStyle {
  width: number;
}

const buildInstructionRow = (
  row: ImageInstructionRow,
  style: ImageRowElementStyle,
): ImageRowElement => {
  const text = row.offset + ') ' + row.tokens.join(' ');
  const textStyle = new Konva.Text({ fontSize: 10 });
  const textSize = textStyle.measureSize(text);
  const height = textSize.height + 40; // + (row.offset % 20);
  return {
    key: row.offset.toString(),
    height,
    element: (
      <Group key={row.offset} draggable>
        <Rect
          width={style.width}
          height={height}
          fill="red"
          shadowBlur={5}
          onClick={() => console.log('click')}
        />
        <Text text={text} width={style.width} height={height} y={5} fill="white" />
      </Group>
    ),
  };
};

const buildRow = (row: ImageBaseRow, style: ImageRowElementStyle): ImageRowElement => {
  if (row.type === ImageRowType.Instruction) {
    return buildInstructionRow(row as ImageInstructionRow, style);
  }
  return {
    key: row.offset.toString(),
    height: 0,
    element: <></>,
  };
};

export interface ImageContentProps {
  imageId: ObjectId;
}

export function ImageContent({ imageId }: ImageContentProps) {
  const viewConfig = {
    scenePaddingRight: 10,
    sliderWidth: 15,
    avgRowHeight: 30,
    wheelDelta: 200,
    cacheSize: 100,
  };
  const stage = useKonvaStage();
  const [totalRowsCount, setTotalRowsCount] = useState(0);
  const [scrollY, setScrollY] = useState(0);
  const [rowsToRender, setRowsToRender] = useState<JSX.Element[]>([]);
  const cachedRows = useRef<{ [idx: number]: ImageRowElement }>({});
  const cachedRowsIdxs = useRef<number[]>([]);
  const stageWidth = stage.size.width - viewConfig.scenePaddingRight;
  const viewWidth = stageWidth - viewConfig.sliderWidth;
  const avgImageContentHeight = viewConfig.avgRowHeight * totalRowsCount;
  const sliderHeight =
    avgImageContentHeight &&
    Math.max(stage.size.height * (stage.size.height / avgImageContentHeight), 20);
  const sliderPosX = stageWidth - viewConfig.sliderWidth;
  const sliderMaxPosY = stage.size.height - sliderHeight;
  const lastRowIdx = totalRowsCount - 1;
  const scrollToRowIdx = (scrollY: number) => (scrollY / sliderMaxPosY) * lastRowIdx;
  const rowIdxToScroll = (rowIdx: number) =>
    (Math.min(Math.max(rowIdx, 0), lastRowIdx) / lastRowIdx) * sliderMaxPosY;
  const scrollRowIdx = scrollToRowIdx(scrollY);
  const curRowIdx = Math.floor(scrollRowIdx);
  const curRowInvisiblePart = scrollRowIdx - curRowIdx;
  const curRowVisiblePart = 1 - curRowInvisiblePart;

  const getRowByIdx = async (rowIdx: number) => {
    let row = cachedRows.current[rowIdx];
    if (!row) {
      const rows = await getImageApi().getImageRowsAt(imageId, rowIdx, 1);
      row = buildRow(rows[0], { width: viewWidth });
      // cache for better performance
      if (cachedRowsIdxs.current.length > viewConfig.cacheSize) {
        const removedIdx = cachedRowsIdxs.current.shift();
        if (removedIdx) {
          delete cachedRows.current[removedIdx];
        }
      }
      cachedRows.current[rowIdx] = row;
      cachedRowsIdxs.current.push(rowIdx);
    }
    return row;
  };

  const getForwardRows = async (totalHeight: number) => {
    const result: { rowIdx: number; y: number; row: ImageRowElement }[] = [];
    let posY = 0;
    let rowIdx = curRowIdx;
    while (rowIdx < totalRowsCount && posY < totalHeight) {
      const row = await getRowByIdx(rowIdx);
      if (rowIdx === curRowIdx) {
        posY -= curRowInvisiblePart * row.height;
      }
      result.push({ rowIdx, y: posY, row });
      posY += row.height;
      rowIdx++;
    }
    return result;
  };

  const getBackwardRows = async (totalHeight: number) => {
    const result: { rowIdx: number; y: number; row: ImageRowElement }[] = [];
    let posY = 0;
    let rowIdx = curRowIdx;
    while (rowIdx >= 0 && posY < totalHeight) {
      const row = await getRowByIdx(rowIdx);
      if (rowIdx === curRowIdx) {
        posY -= curRowVisiblePart * row.height;
      }
      result.push({ rowIdx, y: posY, row });
      posY += row.height;
      rowIdx--;
    }
    return result;
  };

  useEffect(async () => {
    setTotalRowsCount(await getImageApi().getImageTotalRowsCount(imageId));
  }, [imageId]);

  useEffect(async () => {
    setRowsToRender(
      (await getForwardRows(stage.size.height)).map(({ y, row }) => (
        <Group key={row.key} y={y}>
          {row.element}
        </Group>
      )),
    );
  }, [scrollY, stage]);

  const onWheel = useCallback(
    async (e: Konva.KonvaEventObject<WheelEvent>) => {
      if (e.evt.deltaY > 0) {
        const forwardRows = await getForwardRows(viewConfig.wheelDelta);
        if (forwardRows.length > 0) {
          const lastRow = forwardRows[forwardRows.length - 1];
          const newScrollRowIdx =
            lastRow.rowIdx - (lastRow.y - viewConfig.wheelDelta) / lastRow.row.height;
          setScrollY(rowIdxToScroll(newScrollRowIdx));
        }
      } else {
        const backwardRows = await getBackwardRows(viewConfig.wheelDelta);
        if (backwardRows.length > 0) {
          const lastRow = backwardRows[backwardRows.length - 1];
          const newScrollRowIdx =
            lastRow.rowIdx + 1 + (lastRow.y - viewConfig.wheelDelta) / lastRow.row.height;
          setScrollY(rowIdxToScroll(newScrollRowIdx));
        }
      }
    },
    [scrollY, sliderMaxPosY],
  );

  return (
    <>
      <Layer onWheel={onWheel}>
        <Rect width={stage.size.width} height={stage.size.height} />
      </Layer>
      <Layer onWheel={onWheel}>
        <Rect
          width={viewConfig.sliderWidth}
          height={sliderHeight}
          x={sliderPosX}
          y={scrollY}
          fill="green"
          draggable
          dragBoundFunc={(pos) => {
            return {
              x: sliderPosX,
              y: Math.min(Math.max(pos.y, 0), sliderMaxPosY),
            };
          }}
          onDragMove={(e) => {
            setScrollY(e.target.y());
          }}
        />
      </Layer>
      <Layer onWheel={onWheel}>{rowsToRender}</Layer>
    </>
  );
}
