import { Grid, emphasize, useTheme } from '@mui/material';
import { Resizable } from 're-resizable';
import { ObjectId } from 'sda-electron/api/common';
import { getImageApi } from 'sda-electron/api/image';
import { PcodeFunctionGraph, getPcodeApi } from 'sda-electron/api/p-code';
import { IRcodeFunction, getIRcodeApi } from 'sda-electron/api/ir-code';
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
import { PcodeView } from './CodeView/PcodeView';
import { IRcodeView } from './CodeView/IRcodeView';
import { getResearcherApi } from 'sda-electron/api/researcher';

const DecompilerComponent = () => {
  const {
    imageId,
    rowSelection: { selectedRows },
    functions: { goToOffset },
  } = useImageContent();
  const image = useObject(() => getImageApi().getImage(imageId), [imageId]);
  const selectedFirstRow = selectedRows.length > 0 ? selectedRows[0] : null;
  const [curFuncGraph, setCurFuncGraph] = useState<PcodeFunctionGraph | null>(null);
  const [curFunction, setCurFunction] = useState<IRcodeFunction | null>(null);
  const [showIRcode, setShowIRcode] = useState<boolean>(false);
  const [dataFlowText, setDataFlowText] = useState<string>('');
  const [showDataFlowText, setShowDataFlowText] = useState<boolean>(false);
  const [splitIntoColumns, setSplitIntoColumns] = useState<boolean>(true);
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
      const programId = await getIRcodeApi().getProgramIdByImage(image.id);
      const func = await getIRcodeApi().getFunctionAt(programId, curFuncGraph.id.offset);
      setCurFunction(func);
    }),
    [image, curFuncGraph],
  );
  useEffect(
    withCrash_(async () => {
      if (!curFunction) return;
      const dataFlowText = await getResearcherApi().printDataFlowForFunction(curFunction.id);
      setDataFlowText(dataFlowText);
    }),
    [curFunction],
  );
  if (!image) return null;
  return (
    <Grid container direction="column" wrap="nowrap" height="100%">
      <Grid item>
        {imageId.key} <br />
        Decompiler ({selectedRows.length} rows selected) <br />
        <Button variant="contained" onClick={() => goToOffset(image.entryPointOffset)}>
          Go to beginning
        </Button>
        <Button
          variant="contained"
          onClick={() => setShowIRcode(!showIRcode)}
          disabled={!curFunction}
        >
          {showIRcode ? 'Show Pcode' : 'Show IRcode'}
        </Button>
        <Button
          variant="contained"
          onClick={() => setSplitIntoColumns(!splitIntoColumns)}
          disabled={!curFunction}
        >
          {splitIntoColumns ? 'Columns Yes' : 'Columns No'}
        </Button>
        <Button
          variant="contained"
          onClick={() => setShowDataFlowText(!showDataFlowText)}
          disabled={!curFunction}
        >
          {showDataFlowText ? 'Hide Data Flow' : 'Show Data Flow'}
        </Button>
      </Grid>
      <Grid item container flex={1} overflow="auto">
        {showDataFlowText && (
          <Grid item container sx={{ whiteSpace: 'pre-line', userSelect: 'text' }}>
            {dataFlowText}
          </Grid>
        )}
        {showIRcode
          ? curFunction && (
              <IRcodeView image={image} func={curFunction} splitIntoColumns={splitIntoColumns} />
            )
          : curFuncGraph && (
              <PcodeView
                image={image}
                funcGraph={curFuncGraph}
                splitIntoColumns={splitIntoColumns}
              />
            )}
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
            <PopperContextProvider value={popper}>
              <DecompilerComponent />
            </PopperContextProvider>
          </Grid>
        </Resizable>
      </Grid>
      <ContextMenu {...contextMenu.props} />
      <Popper {...popper.props} closeOnMouseLeave />
    </ImageContentProvider>
  );
};
