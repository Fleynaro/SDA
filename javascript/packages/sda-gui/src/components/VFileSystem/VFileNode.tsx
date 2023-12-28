import React, { useCallback, useEffect } from 'react';
import Typography from '@mui/material/Typography';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import FolderOpenOutlinedIcon from '@mui/icons-material/FolderOpenOutlined';
import FolderOutlinedIcon from '@mui/icons-material/FolderOutlined';
import DescriptionIcon from '@mui/icons-material/Description';
import { NodeModel, RenderParams, useDragOver } from '@minoru/react-dnd-treeview';
import { makeStyles } from '@mui/styles';
import { VFile } from 'sda-electron/api/v-file-system';
import { Box, Grid, Input, Theme } from '@mui/material';
import { ContextMenu, useContextMenu } from 'components/Menu';
import { VFileContextMenu } from './VFileContextMenu';
import { useVFileSystem } from './VFileContext';

export type VFileNodeModel = NodeModel<VFile>;

const useStyles = makeStyles((theme: Theme) => ({
  node: {
    '&:hover': {
      backgroundColor: theme.palette.action.hover,
      cursor: 'pointer',
    },
  },

  expandIconWrapper: {
    transition: 'transform linear .1s',
    transform: 'rotate(-90deg)',
  },

  expandIconWrapperOpen: {
    transform: 'rotate(0deg)',
  },

  renameInput: {
    fontSize: '11px',
  },
}));

export interface FileNodeProps {
  node: VFileNodeModel;
  renderParams: RenderParams;
}

export function VFileNode({ node, renderParams }: FileNodeProps) {
  const fileSystem = useVFileSystem();
  const [newName, setNewName] = React.useState<string>('');
  const classes = useStyles();
  const indent = renderParams.depth * 24;
  const file = node.data!;
  const isDirectory = file.type === 'directory';
  const isRenaming = fileSystem.fileToRename === file.path;

  useEffect(() => {
    if (isRenaming) {
      setNewName(file.name);
    }
  }, [setNewName, isRenaming]);

  const rename = () => {
    if (newName.length > 0 && newName !== file.name) {
      fileSystem.renameFile(file, newName);
    }
    fileSystem.setFileToRename(undefined);
  };

  const dragOverProps = useDragOver(node.id, renderParams.isOpen, renderParams.onToggle);

  const contextMenu = useContextMenu();
  const onContextMenu = (e: React.MouseEvent<HTMLElement>) => {
    e.preventDefault();
    contextMenu.open(e);
    contextMenu.setContent(<VFileContextMenu node={node} />);
  };

  return (
    <>
      <Box
        key={node.id.toString()}
        {...dragOverProps}
        onClick={renderParams.onToggle}
        onContextMenu={onContextMenu}
        className={classes.node}
      >
        <Grid
          container
          alignItems="center"
          spacing={1}
          wrap="nowrap"
          style={{ paddingLeft: indent }}
        >
          {isDirectory && (
            <Grid item>
              <ExpandMoreIcon
                className={`${classes.expandIconWrapper} ${
                  renderParams.isOpen ? classes.expandIconWrapperOpen : ''
                }`}
              />
            </Grid>
          )}
          <Grid item>
            {isDirectory ? (
              renderParams.isOpen ? (
                <FolderOpenOutlinedIcon />
              ) : (
                <FolderOutlinedIcon />
              )
            ) : (
              <DescriptionIcon />
            )}
          </Grid>
          <Grid item>
            {isRenaming ? (
              <Input
                autoFocus
                placeholder="Enter name"
                value={newName}
                sx={{ width: '100%', height: '10%' }}
                onChange={(e) => setNewName(e.target.value)}
                onKeyPress={(e) => {
                  if (e.key === 'Enter') {
                    rename();
                  }
                }}
                onBlur={rename}
                className={classes.renameInput}
              />
            ) : (
              <Typography variant="body2">{node.text}</Typography>
            )}
          </Grid>
        </Grid>
      </Box>
      <ContextMenu {...contextMenu.props} />
    </>
  );
}
