import { createContext, useCallback, useContext, useRef, useState } from 'react';
import { useEffect } from 'hooks';
import { ObjectId } from 'sda-electron/api/common';
import {
  getImageApi,
  ImageRowType,
  ImageBaseRow,
  ImageInstructionRow,
} from 'sda-electron/api/image';
import { Group, Layer, Rect, Text } from 'react-konva';
import Konva from 'konva';
import { useKonvaStage, buildKonvaFormatText, useKonvaFormatTextSelection } from 'components/Konva';

interface ImageRowElement {
  offset: number;
  height: number;
  element: JSX.Element;
}

interface RowContextValue {
  selection: {
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

const RowContext = createContext<RowContextValue | null>(null);

const buildInstructionRow = (row: ImageInstructionRow): ImageRowElement => {
  const viewConfig = {
    padding: 5,
    cols: {
      jump: {
        width: 70,
      },
      offset: {
        width: 50,
      },
      instruction: {
        width: 200,
      },
    },
    instructionTokenColors: {
      ['Mneumonic']: '#eddaa4',
      ['Register']: '#93c5db',
      ['Number']: '#e8e8e8',
      ['AddressAbs']: '#d2b2d6',
      ['AddressRel']: '#bdbdbd',
    } as { [type: string]: string },
  };
  const offsetText = buildKonvaFormatText({
    selectionArea: 'offset',
    textIdx: row.offset,
    textParts: [
      {
        text: `0x${row.offset.toString(16)}`,
        style: { fontSize: 12, fill: 'white' },
      },
    ],
    maxWidth: viewConfig.cols.instruction.width,
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
          fill: viewConfig.instructionTokenColors[token.type] || 'white',
        },
      };
    }),
    maxWidth: viewConfig.cols.instruction.width,
    newLineInEnd: true,
  });
  const height = instructionText.height + viewConfig.padding * 2;
  function Elem() {
    const rowCtx = useContext(RowContext);
    if (!rowCtx) return <></>;
    const {
      selection: { selectedRows, firstSelectedRow, setFirstSelectedRow, setLastSelectedRow },
      style: { rowWidth },
    } = rowCtx;
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
        <Group x={viewConfig.padding} y={viewConfig.padding}>
          <Group width={viewConfig.cols.jump.width}></Group>
          <Group
            x={viewConfig.padding + viewConfig.cols.jump.width}
            width={viewConfig.cols.offset.width}
          >
            {offsetText.elem}
          </Group>
          <Group
            x={viewConfig.padding + viewConfig.cols.jump.width + viewConfig.cols.offset.width}
            width={viewConfig.cols.instruction.width}
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

export interface ImageContentProps {
  imageId: ObjectId;
}

export function ImageContent({ imageId }: ImageContentProps) {
  const viewConfig = {
    scenePaddingRight: 10,
    sliderWidth: 15,
    avgRowHeight: 30,
    wheelDelta: 200,
    cacheSize: 200,
  };
  const stage = useKonvaStage();
  const [totalRowsCount, setTotalRowsCount] = useState(0);
  const [scrollY, setScrollY] = useState(0);
  const [rowsToRender, setRowsToRender] = useState<JSX.Element[]>([]);
  const [selectedRows, setSelectedRows] = useState<number[]>([]);
  const [firstSelectedRow, setFirstSelectedRow] = useState<number>();
  const [lastSelectedRow, setLastSelectedRow] = useState<number>();
  const cachedRows = useRef<{ [idx: number]: ImageRowElement }>({});
  const cachedRowsIdxs = useRef<number[]>([]);
  const rowsLoading = useRef(false);
  const textSelection = useKonvaFormatTextSelection();
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
      row = buildRow(rows[0]);
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
    if (rowsLoading.current) return;
    rowsLoading.current = true;
    const rows = await getForwardRows(stage.size.height);
    rowsLoading.current = false;
    setRowsToRender(
      rows.map(({ y, row }) => (
        <Group key={row.offset} y={y}>
          {row.element}
        </Group>
      )),
    );
  }, [scrollY, stage.size.height]);

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

  const onMouseUp = useCallback(() => {
    textSelection.stopSelecting();
  }, [textSelection]);

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
        <RowContext.Provider
          value={{
            selection: {
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
        </RowContext.Provider>
        <Text text={textSelection.selectedTextRef.current} x={10} y={10} fill="green" />
      </Layer>
    </>
  );
}
