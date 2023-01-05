import { useEffect, useCallback, useRef, useState } from 'react';
import { getImageApi } from 'sda-electron/api/image';
import { Group, Layer, Rect, Text } from 'react-konva';
import Konva from 'konva';
import { useKonvaStage, useKonvaFormatTextSelection } from 'components/Konva';
import { ContextMenu, ContextMenuProps, MenuNode } from 'components/Menu';
import { animate } from 'utils';
import { useImageContentStyle } from './style';
import { buildRow, ImageRowElement } from './Row';
import { buildJump } from './Jump';
import { useImageContent } from './context';
import { withCrash, withCrash_ } from 'hooks';

export const ImageContentContextMenu = (props: ContextMenuProps) => {
  return (
    <ContextMenu {...props}>
      <MenuNode
        label="Test"
        onClick={() => {
          console.log('Test');
        }}
      />
    </ContextMenu>
  );
};

export function ImageContent() {
  const stage = useKonvaStage();
  const { styles: style } = useImageContentStyle();
  const {
    imageId,
    setFunctions,
    rowSelection: { firstSelectedRow, lastSelectedRow, setSelectedRows },
  } = useImageContent();
  const [totalRowsCount, setTotalRowsCount] = useState(0);
  const [scrollY, setScrollY] = useState(0);
  const [rowsToRender, setRowsToRender] = useState<JSX.Element[]>([]);
  const [jumpsToRender, setJumpsToRender] = useState<JSX.Element[]>([]);
  const [forceUpdate, setForceUpdate] = useState(0);
  const cachedRows = useRef<{ [idx: number]: ImageRowElement }>({});
  const cachedRowsIdxs = useRef<number[]>([]);
  const sync = useRef({ isLoading: false, requestCount: 0 });
  const textSelection = useKonvaFormatTextSelection();
  const avgImageContentHeight = style.viewport.avgRowHeight * totalRowsCount;
  const sliderHeight =
    avgImageContentHeight &&
    Math.max(stage.size.height * (stage.size.height / avgImageContentHeight), 20);
  const sliderPosX = style.row.width;
  const sliderMaxPosY = stage.size.height - sliderHeight;
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

  const getRowByIdx = async (rowIdx: number) => {
    let row = cachedRows.current[rowIdx];
    if (!row) {
      const rows = await getImageApi().getImageRowsAt(imageId, rowIdx, 1);
      row = buildRow(rows[0], style);
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

  useEffect(
    withCrash_(
      withCrash(async () => {
        setTotalRowsCount(await getImageApi().getImageTotalRowsCount(imageId));
      }),
    ),
    [imageId],
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
    }),
    [imageId, firstSelectedRow, lastSelectedRow],
  );

  // render rows and jumps on scroll and stage resize
  useEffect(
    withCrash_(async () => {
      // rows
      const savedRequestCount = ++sync.current.requestCount;
      if (sync.current.isLoading) return;
      sync.current.isLoading = true;
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
            const jumpElem = buildJump(jump, layerLevel, style, fromY, toY);
            jumpsToRender.push(<Group key={`${jump.from}-${jump.to}`}>{jumpElem}</Group>);
          }
        }
        setJumpsToRender(jumpsToRender);
      }
      sync.current.isLoading = false;
      if (savedRequestCount < sync.current.requestCount) {
        // force component updated to render actual rows
        setForceUpdate(forceUpdate + 1);
      }
    }),
    [scrollY, style, forceUpdate],
  );

  useEffect(() => {
    // clear cache on style change
    cachedRows.current = {};
    cachedRowsIdxs.current = [];
  }, [style]);

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
    [scrollY, sliderMaxPosY, style],
  );

  const goToOffset = useCallback(
    async (offset: number) => {
      const rowIdx = await getImageApi().offsetToRowIdx(imageId, offset);
      if (rowIdx === -1) return false;
      const fromScrollY = scrollY;
      const toScrollY = rowIdxToScroll(rowIdx - rowsToRender.length / 2);
      animate((p) => {
        setScrollY(fromScrollY + (toScrollY - fromScrollY) * p);
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
      <Layer onWheel={onWheel}>
        {rowsToRender}
        {jumpsToRender}
        <Text text={textSelection.selectedTextRef.current} x={10} y={10} fill="green" />
      </Layer>
    </>
  );
}
