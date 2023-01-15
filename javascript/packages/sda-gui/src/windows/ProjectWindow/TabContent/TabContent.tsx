import { Box, emphasize, useTheme } from '@mui/material';
import { Resizable } from 're-resizable';
import { ObjectId } from 'sda-electron/api/common';
import { getImageApi } from 'sda-electron/api/image';
import {
  ImageContent,
  ImageContentContextMenu,
  ImageContentProvider,
  ImageContentStyleBridgeConsumer,
  ImageContentStyleBridgeProvider,
  useImageContent,
} from 'components/NewImageContent';
import {
  KonvaFormatTextSelectionBridgeConsumer,
  KonvaFormatTextSelectionBridgeProvider,
  Stage,
} from 'components/Konva';
import { useContextMenu } from 'components/Menu';
import { useProjectWindowStyles } from '../style';
import Konva from 'konva';
import Button from '@mui/material/Button';
import { useObject } from 'hooks';
import {
  ImageContentBridgeConsumer,
  ImageContentBridgeProvider,
} from 'components/NewImageContent/context';
import { Layer } from 'react-konva';
import { Test } from 'components/Konva/Test';

const DecompilerComponent = () => {
  const {
    imageId,
    rowSelection: { selectedRows },
    functions: { goToOffset },
  } = useImageContent();
  const image = useObject(() => getImageApi().getImage(imageId), [imageId]);
  if (!image) return null;
  return (
    <Box>
      {imageId.key} <br />
      Decompiler ({selectedRows.length} rows selected) <br />
      <Button variant="contained" onClick={() => goToOffset(image.entryPointOffset)}>
        Go to begining
      </Button>
    </Box>
  );
};

export interface TabContentProps {
  imageId: ObjectId;
}

export const TabContent = ({ imageId }: TabContentProps) => {
  const theme = useTheme();
  const classes = useProjectWindowStyles();
  const contextMenu = useContextMenu();

  const onContextMenu = (e: Konva.KonvaEventObject<MouseEvent>) => {
    contextMenu.openAtPos(e.evt.clientX, e.evt.clientY);
  };

  // There's a trouble with context exposing into konva. Solve: https://github.com/konvajs/react-konva/issues/188#issuecomment-478302062
  return (
    <ImageContentProvider imageId={imageId}>
      <KonvaFormatTextSelectionBridgeConsumer>
        {(value1) => (
          <ImageContentStyleBridgeConsumer>
            {(value2) => (
              <ImageContentBridgeConsumer>
                {(value3) => (
                  <Stage
                    sx={{ width: '100%', height: '100%', flexGrow: 1 }}
                    onContextMenu={onContextMenu}
                  >
                    <KonvaFormatTextSelectionBridgeProvider value={value1}>
                      <ImageContentStyleBridgeProvider value={value2}>
                        <ImageContentBridgeProvider value={value3}>
                          <ImageContent />
                          {/* <Layer>
                            <Test />
                          </Layer> */}
                        </ImageContentBridgeProvider>
                      </ImageContentStyleBridgeProvider>
                    </KonvaFormatTextSelectionBridgeProvider>
                  </Stage>
                )}
              </ImageContentBridgeConsumer>
            )}
          </ImageContentStyleBridgeConsumer>
        )}
      </KonvaFormatTextSelectionBridgeConsumer>
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
        <Box
          sx={{
            backgroundColor: emphasize(theme.palette.background.default, 0.05),
            height: '100%',
          }}
        >
          <DecompilerComponent />
        </Box>
      </Resizable>
      <ImageContentContextMenu {...contextMenu.props} />
    </ImageContentProvider>
  );
};
