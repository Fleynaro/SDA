import { useEffect, useCallback, useRef, useState } from 'react';
import { getImageApi, ImageBaseRow } from 'sda-electron/api/image';
import { Group, Layer, Rect, Text } from 'react-konva';
import { Html } from 'react-konva-utils';
import Konva from 'konva';
import { useStage, useTextSelection, Block } from 'components/Konva';
import { ContextMenu, ContextMenuProps, MenuNode } from 'components/Menu';
import { animate } from 'utils';
import { useImageContentStyle } from './style';
import { Row } from './Row';
import { buildJump } from './Jump';
import { useImageContent } from './context';
import { withCrash, withCrash_ } from 'providers/CrashProvider';
import { RenderBlockProps } from 'components/Konva/Block/RenderBlock';
import { PcodeGroup } from 'sda-electron/api/p-code';
import { Box, Button } from '@mui/material';

export const ImageContentContextMenu = (props: ContextMenuProps) => {
  const {
    imageId,
    view,
    setView,
    rowSelection: { selectedRows },
  } = useImageContent();

  const onPCodeAnalysis = useCallback(
    withCrash(async () => {
      if (selectedRows.length === 0) return;
      const startOffset = selectedRows[0];
      await getImageApi().analyzePcode(imageId, [startOffset]);
    }),
    [imageId, selectedRows],
  );

  const onShowPCode = useCallback(() => {
    setView({ ...view, showPcode: !view.showPcode });
  }, [view]);

  return (
    <ContextMenu {...props}>
      <MenuNode label="P-Code Analysis" onClick={onPCodeAnalysis} />
      <MenuNode label={view.showPcode ? 'Hide P-Code' : 'Show P-Code'} onClick={onShowPCode} />
    </ContextMenu>
  );
};

type RowInfo = {
  idx: number;
  offset: number;
  elem: JSX.Element;
  props: RenderBlockProps;
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
  const { selectedAreaType, selectedText, selectedObjects } = useTextSelection();
  const [totalRowsCount, setTotalRowsCount] = useState(0);
  const [scrollY, setScrollY] = useState(0); // [0, 1]
  const [rowsToRender, setRowsToRender] = useState<{
    elem: JSX.Element;
    count: number;
    height: number;
  }>({
    elem: <></>,
    count: 0,
    height: 0,
  });
  const [jumpsToRender, setJumpsToRender] = useState<JSX.Element[]>([]);
  const [forceUpdate, setForceUpdate] = useState(0);
  const cachedRows = useRef<{ [idx: number]: RowInfo }>({});
  const cachedRowsIdxs = useRef<number[]>([]);
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
  const lastRowIdx = totalRowsCount - 1;
  const scrollToRowIdx = useCallback(
    (scrollY: number) => scrollY * lastRowIdx,
    [lastRowIdx, sliderMaxPosY],
  );
  const rowIdxToScroll = useCallback(
    (rowIdx: number) => Math.min(Math.max(rowIdx, 0), lastRowIdx) / lastRowIdx,
    [lastRowIdx, sliderMaxPosY],
  );
  const scrollRowIdx = scrollToRowIdx(scrollY);
  const curRowIdx = Math.floor(scrollRowIdx);
  const curRowInvisiblePart = scrollRowIdx - curRowIdx;
  const curRowVisiblePart = 1 - curRowInvisiblePart;

  const getRowByIdx = async (rowIdx: number, disableCache = false) => {
    let row = cachedRows.current[rowIdx];
    if (!row) {
      const rows = await getImageApi().getImageRowsAt(imageId, rowIdx, 1, {
        tokens: true,
        pcode: view.showPcode,
      });
      const elem = Block({
        key: rowIdx,
        children: <Row rowIdx={rowIdx} row={rows[0]} styles={style} />,
        width: style.row.width,
        textSelection: {
          index: rowIdx,
          area: `image-${imageId.key}`,
        },
      });
      row = {
        idx: rowIdx,
        offset: rows[0].offset,
        elem,
        props: elem.props,
      };
      if (!disableCache) {
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
    }
    return row;
  };

  const getForwardRows = async (totalHeight: number, disableCache = false) => {
    const rows: RowInfo[] = [];
    const y: number[] = [];
    let posY = 0;
    let rowIdx = curRowIdx;
    while (rowIdx < totalRowsCount && posY < totalHeight) {
      const row = await getRowByIdx(rowIdx, disableCache);
      if (rowIdx === curRowIdx) {
        posY -= curRowInvisiblePart * row.props.height;
      }
      rows.push(row);
      y.push(posY);
      posY += row.props.height;
      rowIdx++;
    }
    return { rows, y, height: posY };
  };

  const getBackwardRows = async (totalHeight: number, disableCache = false) => {
    const rows: RowInfo[] = [];
    const y: number[] = [];
    let posY = 0;
    let rowIdx = curRowIdx;
    while (rowIdx >= 0 && posY < totalHeight) {
      const row = await getRowByIdx(rowIdx, disableCache);
      if (rowIdx === curRowIdx) {
        posY -= curRowVisiblePart * row.props.height;
      }
      rows.push(row);
      y.push(posY);
      posY += row.props.height;
      rowIdx--;
    }
    return { rows, y, height: posY };
  };

  const setupRowsAndJumps = async () => {
    const { rows, y, height } = await getForwardRows(stage.size.height);
    if (rows.length > 0) {
      const elem = Block({
        children: rows.map((r) => r.elem),
        flexDir: 'col',
        padding: { top: -rows[0].props.height * curRowInvisiblePart },
      });
      setRowsToRender({
        elem,
        count: rows.length,
        height,
      });

      // jumps
      const startOffset = rows[0].offset;
      const endOffset = rows[rows.length - 1].offset;
      const jumps = await getImageApi().getJumpsAt(imageId, startOffset, endOffset);
      const jumpsToRender: JSX.Element[] = [];
      for (let layerLevel = 0; layerLevel < jumps.length; layerLevel++) {
        const jumpsOnLayer = jumps[layerLevel];
        for (const jump of jumpsOnLayer) {
          const fromRowIdx = rows.findIndex((r) => r.offset === jump.from);
          const toRowIdx = rows.findIndex((r) => r.offset === jump.to);
          const fromY =
            fromRowIdx !== -1 ? y[fromRowIdx] + rows[fromRowIdx].props.height / 2 : undefined;
          const toY = toRowIdx !== -1 ? y[toRowIdx] + rows[toRowIdx].props.height / 2 : undefined;
          const jumpElem = buildJump(jump, layerLevel, style, fromY, toY);
          jumpsToRender.push(<Group key={`${jump.from}-${jump.to}`}>{jumpElem}</Group>);
        }
      }
      setJumpsToRender(jumpsToRender);
    }
  };

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

  // render rows and jumps on scroll
  useEffect(
    withCrash_(async () => {
      // rows
      const savedRequestCount = ++sync.current.requestCount;
      if (sync.current.isLoading) return;
      sync.current.isLoading = true;
      await setupRowsAndJumps();
      sync.current.isLoading = false;
      if (savedRequestCount < sync.current.requestCount) {
        // force component updated to render actual rows
        setForceUpdate(forceUpdate + 1);
      }
    }),
    [scrollY, forceUpdate],
  );

  useEffect(
    withCrash_(async () => {
      // clear cache
      cachedRows.current = {};
      cachedRowsIdxs.current = [];
      await setupRowsAndJumps();
    }),
    [style, view],
  );

  const onWheel = useCallback(
    async (e: Konva.KonvaEventObject<WheelEvent>) => {
      if (e.evt.deltaY > 0) {
        const { rows, y } = await getForwardRows(style.viewport.wheelDelta, true);
        if (rows.length > 0) {
          const lastRow = rows[rows.length - 1];
          const lastRowY = y[y.length - 1];
          const newScrollRowIdx =
            lastRow.idx - (lastRowY - style.viewport.wheelDelta) / lastRow.props.height;
          setScrollY(rowIdxToScroll(newScrollRowIdx));
        }
      } else {
        const { rows, y } = await getBackwardRows(style.viewport.wheelDelta, true);
        if (rows.length > 0) {
          const lastRow = rows[rows.length - 1];
          const lastRowY = y[y.length - 1];
          const newScrollRowIdx =
            lastRow.idx + 1 + (lastRowY - style.viewport.wheelDelta) / lastRow.props.height;
          setScrollY(rowIdxToScroll(newScrollRowIdx));
        }
      }
    },
    [scrollY, sliderMaxPosY, style],
  );

  const onScrollAreaHoverUp = useCallback(
    (e: Konva.KonvaEventObject<MouseEvent>) => {
      if (e.evt.buttons === 1) {
        setScrollY(rowIdxToScroll(scrollRowIdx - 0.2));
      } else {
        // ignore this event
        e.cancelBubble = true;
      }
    },
    [scrollRowIdx],
  );

  const onScrollAreaHoverDown = useCallback(
    (e: Konva.KonvaEventObject<MouseEvent>) => {
      if (e.evt.buttons === 1) {
        setScrollY(rowIdxToScroll(scrollRowIdx + 0.2));
      }
    },
    [scrollRowIdx],
  );

  const goToOffset = useCallback(
    async (offset: number) => {
      const targetRowIdx = await getImageApi().offsetToRowIdx(imageId, offset);
      if (targetRowIdx === -1) return false;
      const targetRow = (await getImageApi().getImageRowsAt(imageId, targetRowIdx, 1))[0];
      const fromScrollY = scrollY;
      const toScrollY = rowIdxToScroll(targetRowIdx - rowsToRender.count / 2);
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
        {rowsToRender.elem}
        {jumpsToRender}
        <Text text={selectedText} x={700} y={10} fill="green" />
        {selectedAreaType === 'pcode' && (
          <Text
            text={selectedObjects
              .map((obj) => {
                const action = (obj as PcodeGroup).action;
                if (action.name !== 'instruction') return '';
                return `0x${action.offset.toString(16)}`;
              })
              .join('\n')}
            x={1000}
            y={10}
            fill="red"
          />
        )}
      </Layer>
      <Layer onWheel={onWheel}>
        <Rect width={stage.size.width} height={10} onMouseMove={onScrollAreaHoverUp} />
        <Rect
          y={stage.size.height - 10}
          width={stage.size.width}
          height={10}
          onMouseMove={onScrollAreaHoverDown}
        />
        <Rect
          width={style.viewport.sliderWidth}
          height={sliderHeight}
          x={sliderPosX}
          y={scrollY * sliderMaxPosY}
          fill="#545454"
          draggable
          dragBoundFunc={(pos) => {
            return {
              x: sliderPosX,
              y: Math.min(Math.max(pos.y, 0), sliderMaxPosY),
            };
          }}
          onDragMove={(e) => {
            setScrollY(e.target.y() / sliderMaxPosY);
          }}
        />
        <Group x={200} y={200}>
          <Html>
            {/* <Button onClick={() => console.log('click!')} variant="contained">
              Click me
            </Button> */}
            <Box display="flex" flexDirection="column">
              <Button onClick={() => goToOffset(0x1000)} variant="contained" size="small">
                Go to 0x1000
              </Button>
              <Button onClick={() => goToOffset(0x2000)} variant="contained" size="small">
                Go to 0x2000
              </Button>
            </Box>
          </Html>
        </Group>
      </Layer>
    </>
  );
}
