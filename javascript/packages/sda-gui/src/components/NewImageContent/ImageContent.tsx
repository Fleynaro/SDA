import { useEffect, useCallback, useRef, useState } from 'react';
import { getImageApi, ImageBaseRow, ImageLoadRowOptions } from 'sda-electron/api/image';
import { Group, Layer, Rect, Text } from 'react-konva';
import Konva from 'konva';
import { useStage, useKonvaFormatTextSelection, Block, BlockRef } from 'components/Konva';
import { ContextMenu, ContextMenuProps, MenuNode } from 'components/Menu';
import { animate } from 'utils';
import { useImageContentStyle } from './style';
import { useImageContent } from './context';
import { withCrash, withCrash_ } from 'providers/CrashProvider';
import { Row } from './Row';
import { ObjectId } from 'sda-electron/api/common';

export const ImageContentContextMenu = (props: ContextMenuProps) => {
  return <></>;
};

export function ImageContent() {
  const stage = useStage();
  const { styles: style } = useImageContentStyle();
  const {
    imageId,
    setFunctions,
    view,
    rowSelection: { firstSelectedRow, lastSelectedRow, setSelectedRows },
  } = useImageContent();
  const [totalRowsCount, setTotalRowsCount] = useState(0);
  const [scrollY, setScrollY] = useState(0);
  const [rowsToRender, setRowsToRender] = useState<JSX.Element[]>([]);
  const [rowsHeight, setRowsHeight] = useState<number>(0);
  const [jumpsToRender, setJumpsToRender] = useState<JSX.Element[]>([]);
  const [forceUpdate, setForceUpdate] = useState(0);
  //const cachedRows = useRef<{ [idx: number]: ImageRowElement }>({});
  //const cachedRowsIdxs = useRef<number[]>([]);
  const sync = useRef({ isLoading: false, requestCount: 0 });
  const avgImageContentHeight = style.viewport.avgRowHeight * totalRowsCount;
  const sliderHeight =
    avgImageContentHeight &&
    Math.min(
      Math.max(stage.size.height * (stage.size.height / avgImageContentHeight), 20),
      stage.size.height * 0.9,
    );
  const sliderPosX = style.row.width;
  const sliderMaxPosY = stage.size.height - sliderHeight;
  const allRowsOnScreen = rowsHeight >= stage.size.height;
  const lastRowIdx = totalRowsCount - 1;
  const scrollToRowIdx = useCallback(
    (scrollY: number) => (scrollY / sliderMaxPosY) * lastRowIdx,
    [lastRowIdx, sliderMaxPosY],
  );
  const rowIdxToScroll = useCallback(
    (rowIdx: number) => (Math.min(Math.max(rowIdx, 0), lastRowIdx) / lastRowIdx) * sliderMaxPosY,
    [lastRowIdx, sliderMaxPosY],
  );
  const scrollRowIdx = scrollToRowIdx(scrollY);
  const curRowIdx = Math.floor(scrollRowIdx);
  const curRowInvisiblePart = scrollRowIdx - curRowIdx;
  const curRowVisiblePart = 1 - curRowInvisiblePart;

  // NEW
  const [rows, setRows] = useState<ImageBaseRow[]>([]);
  const cachedRows = useRef<{ [idx: number]: ImageBaseRow }>({});
  const cachedRowsIdxs = useRef<number[]>([]);
  const rowsBlockRef = useRef<BlockRef>(null);
  const rowsBlockSize = rowsBlockRef.current?.getParentSize();
  const firstRowHeight = rowsBlockRef.current?.getSize(0).height || 0;

  const cacheRow = useCallback(
    (rowIdx: number, row: ImageBaseRow) => {
      if (cachedRows.current[rowIdx]) return;
      if (cachedRowsIdxs.current.length > style.viewport.cacheSize) {
        const removedIdx = cachedRowsIdxs.current.shift();
        if (removedIdx) {
          delete cachedRows.current[removedIdx];
        }
      }
      cachedRows.current[rowIdx] = row;
      cachedRowsIdxs.current.push(rowIdx);
    },
    [cachedRows, cachedRowsIdxs],
  );

  const getImageRowsAt = useCallback(
    async (imageId: ObjectId, rowIdx: number, count: number, opts?: ImageLoadRowOptions) => {
      let result: ImageBaseRow[] = [];
      count = Math.min(count, totalRowsCount - rowIdx);
      for (let i = 0; i < count; i++) {
        const row = cachedRows.current[rowIdx + i];
        if (row) {
          result.push(row);
        } else {
          break;
        }
      }
      if (result.length < count) {
        result = await getImageApi().getImageRowsAt(imageId, rowIdx, count, opts);
        result.forEach((row, i) => {
          cacheRow(rowIdx + i, row);
        });
      }
      return result;
    },
    [totalRowsCount],
  );

  useEffect(
    withCrash_(async () => {
      setTotalRowsCount(await getImageApi().getImageTotalRowsCount(imageId));
    }),
    [],
  );

  // select one or more rows with scrolling if needed
  useEffect(
    withCrash_(async () => {
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
          // todo: вычислить allRowsOnScreen и rowsToRender
          // if (allRowsOnScreen) {
          //   if (lastRowIdx >= scrollRowIdx + rowsToRender.length - 1 - 2) {
          //     setScrollY(rowIdxToScroll(scrollRowIdx + 1));
          //   }
          // }
        }
        const newSelectedRows: number[] = [];
        for (let i = firstRowIdx; i <= lastRowIdx; i++) {
          const row = await getImageRowsAt(imageId, i, 1);
          newSelectedRows.push(row[0].offset);
        }
        setSelectedRows(newSelectedRows);
      }
    }),
    [imageId, firstSelectedRow, lastSelectedRow],
  );

  //console.log('rowsBlockRef', rowsBlockSize);

  useEffect(
    withCrash_(async () => {
      const savedRequestCount = ++sync.current.requestCount;
      if (sync.current.isLoading) return;
      sync.current.isLoading = true;
      const newRows = await getImageRowsAt(imageId, curRowIdx, 50, {
        tokens: true,
        pcode: view.showPcode,
      });
      setRows(newRows);
      sync.current.isLoading = false;
      if (savedRequestCount < sync.current.requestCount) {
        // force component updated to render actual rows
        setForceUpdate(forceUpdate + 1);
      }
    }),
    [curRowIdx],
  );

  // useEffect(
  //   withCrash_(async () => {
  //     if (!rowsBlockSize || rowsBlockSize.height > stage.size.height) return;
  //     const newRows = await getImageRowsAt(imageId, rows.length, 5, {
  //       tokens: true,
  //       pcode: view.showPcode,
  //     });
  //     setRows((prev) => [...prev, ...newRows]);
  //   }),
  //   [rowsBlockSize?.height, stage.size.height],
  // );

  const onWheel = useCallback(
    async (e: Konva.KonvaEventObject<WheelEvent>) => {
      if (!rowsBlockRef.current) return;
      if (e.evt.deltaY > 0) {
        // let lastPosY = 0;
        // let lastHeight = 0;
        // let rowIdx = curRowIdx;
        // while (rowIdx < totalRowsCount && lastPosY + lastHeight < style.viewport.wheelDelta) {
        //   lastPosY += lastHeight;
        //   const rowSize = rowsBlockRef.current?.getSize(rowIdx - curRowIdx);
        //   if (rowIdx === curRowIdx) {
        //     lastPosY -= curRowInvisiblePart * rowSize.height;
        //   }
        //   lastHeight = rowSize.height;
        //   rowIdx++;
        // }
        // const newScrollRowIdx = rowIdx - 1 - (lastPosY - style.viewport.wheelDelta) / lastHeight;
        // setScrollY(rowIdxToScroll(newScrollRowIdx));
        setScrollY(rowIdxToScroll(curRowIdx + 5));
      } else {
        // todo: рендерить задние строки и повторить логику выше (e.evt.deltaY > 0)
        setScrollY(rowIdxToScroll(curRowIdx - 5));
      }
    },
    [scrollY, sliderMaxPosY, totalRowsCount, style],
  );

  const goToOffset = useCallback(
    async (offset: number) => {
      const targetRowIdx = await getImageApi().offsetToRowIdx(imageId, offset);
      if (targetRowIdx === -1) return false;
      const targetRow = (await getImageApi().getImageRowsAt(imageId, targetRowIdx, 1))[0];
      const fromScrollY = scrollY;
      const toScrollY = rowIdxToScroll(targetRowIdx - rowsToRender.length / 2); // todo: rowsToRender
      animate((p) => {
        setScrollY(fromScrollY + (toScrollY - fromScrollY) * p);
        if (p >= 1) {
          setSelectedRows([targetRow.offset]);
        }
      }, 1000);
      return true;
    },
    [imageId, scrollY, rowsToRender, rowIdxToScroll],
  );

  useEffect(() => {
    setFunctions({
      goToOffset,
    });
  }, [goToOffset]);

  return (
    <>
      <Layer onWheel={onWheel}>
        <Rect width={stage.size.width} height={stage.size.height} />
      </Layer>
      <Layer onWheel={onWheel}>
        <Block
          width={1000}
          ref={rowsBlockRef}
          onUpdate={() => setForceUpdate((prev) => prev + 1)}
          padding={{ top: -firstRowHeight * curRowInvisiblePart }} // todo: optimize by making parent block with padding
        >
          {rows.map((row, i) => (
            <Row key={row.offset} idx={i} row={row} /> // todo: key = row.offset vs i
          ))}
        </Block>
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
    </>
  );
}
