import { Box, Grid, emphasize, useTheme } from '@mui/material';
import { Resizable } from 're-resizable';
import { ObjectId, TokenizedText } from 'sda-electron/api/common';
import { getImageApi } from 'sda-electron/api/image';
import { PcodeFunctionGraph, getPcodeApi } from 'sda-electron/api/p-code';
import {
  ImageContent,
  ImageContentContextMenu,
  ImageContentProvider,
  ImageContentStyleBridgeConsumer,
  ImageContentStyleBridgeProvider,
  useImageContent,
} from 'components/ImageContent';
import {
  Block,
  KonvaTextSelectionBridgeConsumer,
  KonvaTextSelectionBridgeProvider,
  Stage,
} from 'components/Konva';
import { ContextMenu, useContextMenu } from 'components/Menu';
import { Popper, usePopper, PopperContextProvider } from 'components/Popper';
import { useProjectWindowStyles } from '../style';
import Konva from 'konva';
import Button from '@mui/material/Button';
import { useObject } from 'hooks';
import {
  ImageContentBridgeConsumer,
  ImageContentBridgeProvider,
} from 'components/ImageContent/context';
import { Test2 } from 'components/Konva/Block/Test';
import { useEffect, useState } from 'react';
import { withCrash_ } from 'providers/CrashProvider';
import { HtmlTextSelectionBridgeConsumer, HtmlTextSelectionBridgeProvider } from 'components/Text';

const DecompilerComponent = () => {
  const {
    imageId,
    rowSelection: { selectedRows },
    functions: { goToOffset },
  } = useImageContent();
  const image = useObject(() => getImageApi().getImage(imageId), [imageId]);
  const selectedFirstRow = selectedRows.length > 0 ? selectedRows[0] : null;
  const [curFuncGraph, setCurFuncGraph] = useState<PcodeFunctionGraph | null>(null);
  const [text, setText] = useState<TokenizedText | null>(null);
  useEffect(
    withCrash_(async () => {
      if (!image || !selectedFirstRow) return;
      const offset = selectedFirstRow;
      const graphId = await getPcodeApi().getGraphIdByImage(image.id);
      const block = await getPcodeApi().getBlockAt(graphId, offset, false);
      if (!block) return;
      setCurFuncGraph(block.functionGraph);
    }),
    [image, selectedFirstRow],
  );
  useEffect(
    withCrash_(async () => {
      if (!image || !curFuncGraph) return;
      const tokenizedText = await getPcodeApi().getPcodeTokenizedText(
        image.contextId,
        curFuncGraph.id,
      );
      setText(tokenizedText);
    }),
    [image, curFuncGraph?.id.offset],
  );
  if (!image) return null;
  return (
    <Grid container direction="column" wrap="nowrap" height="100%">
      <Grid item>
        {imageId.key} <br />
        Decompiler ({selectedRows.length} rows selected) <br />
        <Button variant="contained" onClick={() => goToOffset(image.entryPointOffset)}>
          Go to begining
        </Button>
        <Button variant="contained" onClick={() => setText(null)}>
          Clear
        </Button>
      </Grid>
      <Grid item container flex={1} overflow="auto">
        {text && <TokenizedTextView text={text} />}
      </Grid>
    </Grid>
  );
};

const TestComponent = () => {
  // t123
  const tree = Block({ children: <Test2 /> });
  console.log(tree);
  return tree;
};

export interface TabContentProps {
  imageId: ObjectId;
}

export const TabContent = ({ imageId }: TabContentProps) => {
  const theme = useTheme();
  const classes = useProjectWindowStyles();
  const contextMenu = useContextMenu();
  const popper = usePopper();

  const onContextMenu = (e: Konva.KonvaEventObject<MouseEvent>) => {
    contextMenu.openAtPos(e.evt.clientX, e.evt.clientY);
    contextMenu.setContent(<ImageContentContextMenu />);
  };

  // There's a trouble with context exposing into konva. Solve: https://github.com/konvajs/react-konva/issues/188#issuecomment-478302062
  return (
    <ImageContentProvider imageId={imageId}>
      <Grid container height="100%" direction="row" wrap="nowrap">
        <Grid item container flex={1}>
          <HtmlTextSelectionBridgeConsumer>
            {(value1) => (
              <KonvaTextSelectionBridgeConsumer>
                {(value2) => (
                  <ImageContentStyleBridgeConsumer>
                    {(value3) => (
                      <ImageContentBridgeConsumer>
                        {(value4) => (
                          <Stage
                            sx={{ width: '100%', height: '100%', flexGrow: 1 }}
                            onContextMenu={onContextMenu}
                          >
                            <HtmlTextSelectionBridgeProvider value={value1}>
                              <KonvaTextSelectionBridgeProvider value={value2}>
                                <ImageContentStyleBridgeProvider value={value3}>
                                  <ImageContentBridgeProvider value={value4}>
                                    <PopperContextProvider value={popper}>
                                      {/* <Layer>
                                    <TestComponent />
                                  </Layer> */}
                                      <ImageContent />
                                      {/* <Layer>
                                    <Test />
                                  </Layer> */}
                                    </PopperContextProvider>
                                  </ImageContentBridgeProvider>
                                </ImageContentStyleBridgeProvider>
                              </KonvaTextSelectionBridgeProvider>
                            </HtmlTextSelectionBridgeProvider>
                          </Stage>
                        )}
                      </ImageContentBridgeConsumer>
                    )}
                  </ImageContentStyleBridgeConsumer>
                )}
              </KonvaTextSelectionBridgeConsumer>
            )}
          </HtmlTextSelectionBridgeConsumer>
        </Grid>
        <Resizable
          defaultSize={{
            width: 200,
            height: 'auto',
          }}
          enable={{ left: true }}
          handleClasses={{
            left: classes.resizable,
          }}
        >
          <Grid
            item
            container
            height="100%"
            sx={{
              backgroundColor: emphasize(theme.palette.background.default, 0.05),
            }}
          >
            <DecompilerComponent />
          </Grid>
        </Resizable>
      </Grid>
      <ContextMenu {...contextMenu.props} />
      <Popper {...popper.props} closeOnMouseLeave />
    </ImageContentProvider>
  );
};
