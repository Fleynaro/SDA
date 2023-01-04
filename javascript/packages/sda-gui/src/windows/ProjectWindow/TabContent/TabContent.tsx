import { Box, emphasize, useTheme } from '@mui/material';
import { Resizable } from 're-resizable';
import { ObjectId } from 'sda-electron/api/common';
import {
  ImageContent,
  ImageContentContextMenu,
  ImageContentProvider,
  ImageContentStyleProvider,
  useImageContent,
} from 'components/ImageContent';
import { KonvaStage } from 'components/Konva';
import { useContextMenu } from 'components/Menu';
import { useProjectWindowStyles } from '../style';
import Konva from 'konva';
import Button from '@mui/material/Button';

const DecompilerComponent = () => {
  const {
    rowSelection: { selectedRows },
    functions: { goToOffset },
  } = useImageContent();
  return (
    <Box>
      Decompiler ({selectedRows.length} rows selected) <br />
      <Button variant="contained" onClick={() => goToOffset(0x1000)}>
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

  return (
    <ImageContentProvider imageId={imageId}>
      <KonvaStage sx={{ width: '100%', height: '100%', flexGrow: 1 }} onContextMenu={onContextMenu}>
        <ImageContent />
      </KonvaStage>
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
