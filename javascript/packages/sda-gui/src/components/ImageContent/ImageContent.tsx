import { createContext, useCallback, useContext, useRef, useState } from 'react';
import { useEffect } from 'hooks';
import { ObjectId } from 'sda-electron/api/common';
import {
  getImageApi,
  ImageRowType,
  ImageBaseRow,
  ImageInstructionRow,
  Jump,
} from 'sda-electron/api/image';
import { Arrow, Group, Layer, Rect, Text } from 'react-konva';
import Konva from 'konva';
import {
  useKonvaStage,
  buildKonvaFormatText,
  useKonvaFormatTextSelection,
  setCursor,
} from 'components/Konva';
import style from './style';

interface ImageRowElement {
  offset: number;
  height: number;
  element: JSX.Element;
}

interface ImageContentContextValue {
  goToOffset: (offset: number) => void;
  rowSelection: {
    selectedRows: number[];
    setSelectedRows: (rows: number[]) => void;
    firstSelectedRow?: number;
    setFirstSelectedRow?: (row?: number) => void;
    lastSelectedRow?: number;
    setLastSelectedRow?: (row?: number) => void;
  };
  style: {
    rowWidth: number;
  };
}

const ImageContentContext = createContext<ImageContentContextValue | null>(null);

const useImageContentContext = () => {
  const ctx = useContext(ImageContentContext);
  if (!ctx) throw new Error('ImageContentContext is not set');
  return ctx;
};

const buildInstructionRow = (row: ImageInstructionRow): ImageRowElement => {
  const offsetText = buildKonvaFormatText({
    selectionArea: 'offset',
    textIdx: row.offset,
    textParts: [
      {
        text: `0x${row.offset.toString(16)}`,
        style: { fontSize: 12, fill: 'white' },
      },
    ],
    maxWidth: style.row.cols.instruction.width,
    newLineInEnd: true,
  });
  const instructionText = buildKonvaFormatText({
    selectionArea: 'instruction',
    textIdx: row.offset,
    textParts: row.tokens.map((token) => {
      return {
        text: token.text,
        style: {
          fontSize: 12,
          fill: style.row.instructionTokenColors[token.type] || 'white',
        },
      };
    }),
    maxWidth: style.row.cols.instruction.width,
    newLineInEnd: true,
  });
  const height = instructionText.height + style.row.padding * 2;
  function Elem() {
    const {
      rowSelection: { selectedRows, firstSelectedRow, setFirstSelectedRow, setLastSelectedRow },
      style: { rowWidth },
    } = useImageContentContext();
    const isSelected = selectedRows.includes(row.offset);

    const onMouseDown = useCallback(() => {
      setFirstSelectedRow?.(row.offset);
    }, [setFirstSelectedRow]);

    const onMouseMove = useCallback(() => {
      if (firstSelectedRow) {
        setLastSelectedRow?.(row.offset);
      }
    }, [firstSelectedRow, setFirstSelectedRow]);

    const onMouseUp = useCallback(() => {
      setFirstSelectedRow?.(undefined);
      setLastSelectedRow?.(undefined);
    }, [setFirstSelectedRow, setLastSelectedRow]);

    return (
      <Group
        width={rowWidth}
        onMouseDown={onMouseDown}
        onMouseMove={onMouseMove}
        onMouseUp={onMouseUp}
      >
        <Rect
          width={rowWidth}
          height={height}
          fill={isSelected ? '#360b0b' : 'black'}
          shadowBlur={5}
        />
        <Group x={style.row.padding} y={style.row.padding}>
          <Group width={style.row.cols.jump.width}></Group>
          <Group
            x={style.row.padding + style.row.cols.jump.width}
            width={style.row.cols.offset.width}
          >
            {offsetText.elem}
          </Group>
          <Group
            x={style.row.padding + style.row.cols.jump.width + style.row.cols.offset.width}
            width={style.row.cols.instruction.width}
          >
            {instructionText.elem}
          </Group>
        </Group>
      </Group>
    );
  }
  return {
    offset: row.offset,
    height,
    element: <Elem />,
  };
};

const buildRow = (row: ImageBaseRow): ImageRowElement => {
  if (row.type === ImageRowType.Instruction) {
    return buildInstructionRow(row as ImageInstructionRow);
  }
  return {
    offset: row.offset,
    height: 0,
    element: <></>,
  };
};

const buildJump = (jump: Jump, layerLevel: number, fromY?: number, toY?: number): JSX.Element => {
  const direction = jump.from < jump.to ? 1 : -1;
  const startY = fromY || -10000 * direction; // 10000 = endless
  const endY = toY || 10000 * direction;
  const isClickable = fromY === undefined || toY === undefined;
  const startX = style.row.cols.jump.width;
  const layerOffset = style.jump.pointerSize + layerLevel * style.jump.distanceBetweenLayers;
  const p1 = [startX + style.jump.pointerSize, startY];
  const p2 = [startX - layerOffset, p1[1]];
  const p3 = [p2[0], endY];
  const p4 = [p1[0], p3[1]];
  function Elem() {
    const { goToOffset } = useImageContentContext();

    const onClickJump = useCallback(() => {
      if (fromY === undefined) {
        goToOffset(jump.from);
      } else if (toY === undefined) {
        goToOffset(jump.to);
      }
    }, []);

    const onMouseEnter = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      if (!isClickable) return;
      const target = e.target as Konva.Arrow;
      target.fill(style.jump.hoverColor);
      target.stroke(style.jump.hoverColor);
      setCursor(e, 'pointer');
    }, []);

    const onMouseLeave = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      if (!isClickable) return;
      const target = e.target as Konva.Arrow;
      target.fill(style.jump.color);
      target.stroke(style.jump.color);
      setCursor(e, 'default');
    }, []);

    return (
      <Arrow
        points={[p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1]]}
        pointerLength={style.jump.pointerSize}
        pointerWidth={style.jump.pointerSize}
        fill={style.jump.color}
        stroke={style.jump.color}
        strokeWidth={style.jump.arrowWidth}
        onClick={onClickJump}
        onMouseEnter={onMouseEnter}
        onMouseLeave={onMouseLeave}
      />
    );
  }
  return <Elem />;
};

export interface ImageContentProps {
  imageId: ObjectId;
}

export function ImageContent({ imageId }: ImageContentProps) {
  const stage = useKonvaStage();
  const [totalRowsCount, setTotalRowsCount] = useState(0);
  const [scrollY, setScrollY] = useState(0);
  const [rowsToRender, setRowsToRender] = useState<JSX.Element[]>([]);
  const [jumpsToRender, setJumpsToRender] = useState<JSX.Element[]>([]);
  const [selectedRows, setSelectedRows] = useState<number[]>([]);
  const [firstSelectedRow, setFirstSelectedRow] = useState<number>();
  const [lastSelectedRow, setLastSelectedRow] = useState<number>();
  const cachedRows = useRef<{ [idx: number]: ImageRowElement }>({});
  const cachedRowsIdxs = useRef<number[]>([]);
  const rowsLoading = useRef(false);
  const textSelection = useKonvaFormatTextSelection();
  const stageWidth = stage.size.width - style.viewport.scenePaddingRight;
  const viewWidth = stageWidth - style.viewport.sliderWidth;
  const avgImageContentHeight = style.viewport.avgRowHeight * totalRowsCount;
  const sliderHeight =
    avgImageContentHeight &&
    Math.max(stage.size.height * (stage.size.height / avgImageContentHeight), 20);
  const sliderPosX = stageWidth - style.viewport.sliderWidth;
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
      row = buildRow(rows[0]);
      // cache for better performance
      if (cachedRowsIdxs.current.length > style.viewport.cacheSize) {
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

  // select one or more rows with scrolling if needed
  useEffect(async () => {
    if (firstSelectedRow !== undefined) {
      let firstRowIdx = await getImageApi().offsetToRowIdx(imageId, firstSelectedRow);
      let lastRowIdx = await getImageApi().offsetToRowIdx(
        imageId,
        lastSelectedRow !== undefined ? lastSelectedRow : firstSelectedRow,
      );
      if (firstRowIdx > lastRowIdx) {
        [firstRowIdx, lastRowIdx] = [lastRowIdx, firstRowIdx];
        if (firstRowIdx <= scrollRowIdx + 2) {
          setScrollY(rowIdxToScroll(scrollRowIdx - 1));
        }
      } else {
        if (lastRowIdx >= scrollRowIdx + rowsToRender.length - 1 - 2) {
          setScrollY(rowIdxToScroll(scrollRowIdx + 1));
        }
      }
      const newSelectedRows: number[] = [];
      for (let i = firstRowIdx; i <= lastRowIdx; i++) {
        const row = await getImageApi().getImageRowsAt(imageId, i, 1);
        newSelectedRows.push(row[0].offset);
      }
      setSelectedRows(newSelectedRows);
    }
  }, [imageId, firstSelectedRow, lastSelectedRow]);

  // render rows on scroll and stage resize
  useEffect(async () => {
    // rows
    if (rowsLoading.current) return;
    rowsLoading.current = true;
    const rows = await getForwardRows(stage.size.height);
    setRowsToRender(
      rows.map(({ y, row }) => (
        <Group key={row.offset} y={y}>
          {row.element}
        </Group>
      )),
    );
    // jumps
    if (rows.length > 0) {
      const startOffset = rows[0].row.offset;
      const endOffset = rows[rows.length - 1].row.offset;
      const jumps = await getImageApi().getJumpsAt(imageId, startOffset, endOffset);
      const jumpsToRender: JSX.Element[] = [];
      for (let layerLevel = 0; layerLevel < jumps.length; layerLevel++) {
        const jumpsOnLayer = jumps[layerLevel];
        for (const jump of jumpsOnLayer) {
          const fromRow = rows.find((r) => r.row.offset === jump.from);
          const toRow = rows.find((r) => r.row.offset === jump.to);
          const fromY = fromRow && fromRow.y + fromRow.row.height / 2;
          const toY = toRow && toRow.y + toRow.row.height / 2;
          const jumpElem = buildJump(jump, layerLevel, fromY, toY);
          jumpsToRender.push(<Group key={`${jump.from}-${jump.to}`}>{jumpElem}</Group>);
        }
      }
      setJumpsToRender(jumpsToRender);
    }
    rowsLoading.current = false;
  }, [scrollY, stage.size.height]);

  const onWheel = useCallback(
    async (e: Konva.KonvaEventObject<WheelEvent>) => {
      if (e.evt.deltaY > 0) {
        const forwardRows = await getForwardRows(style.viewport.wheelDelta);
        if (forwardRows.length > 0) {
          const lastRow = forwardRows[forwardRows.length - 1];
          const newScrollRowIdx =
            lastRow.rowIdx - (lastRow.y - style.viewport.wheelDelta) / lastRow.row.height;
          setScrollY(rowIdxToScroll(newScrollRowIdx));
        }
      } else {
        const backwardRows = await getBackwardRows(style.viewport.wheelDelta);
        if (backwardRows.length > 0) {
          const lastRow = backwardRows[backwardRows.length - 1];
          const newScrollRowIdx =
            lastRow.rowIdx + 1 + (lastRow.y - style.viewport.wheelDelta) / lastRow.row.height;
          setScrollY(rowIdxToScroll(newScrollRowIdx));
        }
      }
    },
    [scrollY, sliderMaxPosY],
  );

  const onMouseUp = useCallback(() => {
    textSelection.stopSelecting();
  }, [textSelection]);

  const goToOffset = useCallback(
    async (offset: number) => {
      const rowIdx = await getImageApi().offsetToRowIdx(imageId, offset);
      setScrollY(rowIdxToScroll(rowIdx - rowsToRender.length / 2));
    },
    [imageId, rowIdxToScroll],
  );

  return (
    <>
      <Layer onWheel={onWheel}>
        <Rect width={stage.size.width} height={stage.size.height} />
      </Layer>
      <Layer onWheel={onWheel}>
        <Rect
          width={style.viewport.sliderWidth}
          height={sliderHeight}
          x={sliderPosX}
          y={scrollY}
          fill="#545454"
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
      <Layer onWheel={onWheel} onMouseUp={onMouseUp}>
        <ImageContentContext.Provider
          value={{
            goToOffset,
            rowSelection: {
              selectedRows,
              setSelectedRows,
              firstSelectedRow,
              setFirstSelectedRow,
              lastSelectedRow,
              setLastSelectedRow,
            },
            style: {
              rowWidth: viewWidth,
            },
          }}
        >
          {rowsToRender}
          {jumpsToRender}
        </ImageContentContext.Provider>
        <Text text={textSelection.selectedTextRef.current} x={10} y={10} fill="green" />
      </Layer>
    </>
  );
}
