import { useCallback, useEffect, useRef, useState } from 'react';
import { DndProvider } from 'react-dnd';
import {
  Tree,
  MultiBackend,
  getBackendOptions,
  DropOptions,
  TreeMethods,
} from '@minoru/react-dnd-treeview';
import { Theme } from '@mui/material';
import { makeStyles } from '@mui/styles';
import { VFileNode, VFileNodeModel } from './VFileNode';
import { FileClassName, VFile, getVFileSystemApi } from 'sda-electron/api/v-file-system';
import { withCrash, withCrash_ } from 'providers/CrashProvider';
import { getEventApi } from 'sda-electron/api/event';
import { VFileSystemProvider } from './VFileContext';

const useStyles = makeStyles((theme: Theme) => ({
  root: {
    paddingLeft: 0,
    '& ul': {
      paddingLeft: 0,
    },
  },

  draggingSource: {
    opacity: '.3',
  },

  dropTarget: {
    backgroundColor: theme.palette.action.hover,
  },
}));

export interface VFileSystemProps {
  onFileOpened?: (file: VFile) => void;
}

export function VFileSystem({ onFileOpened }: VFileSystemProps) {
  const classes = useStyles();
  const treeRef = useRef<TreeMethods>(null);
  const [treeData, setTreeData] = useState<VFileNodeModel[]>([]);
  const [fileToRename, setFileToRename] = useState<string | undefined>(undefined);

  // handle drop event to move file from one directory to another
  const handleDrop = withCrash(async (tree: VFileNodeModel[], options: DropOptions<VFile>) => {
    const { dragSource, dropTarget } = options;
    if (dragSource?.data && dropTarget?.data) {
      const srcPath = dragSource.data.path;
      const dstDirPath = dropTarget.data.path;
      if (srcPath !== '/') {
        const srcPathParts = srcPath.split('/');
        await getVFileSystemApi().moveFile(
          srcPath,
          `${dstDirPath}/${srcPathParts[srcPathParts.length - 1]}`,
        );
      }
    }
  });

  const buildTree = async (nodes: VFileNodeModel[], file: VFile, parent?: VFile) => {
    nodes.push({
      id: file.id,
      parent: parent?.id || 0,
      text: file.name || '/',
      data: file,
      droppable: file.type === 'directory',
    });
    if (file.type === 'directory') {
      const sortedChildPaths = file.files.sort((a, b) => a.localeCompare(b));
      for (const childPath of sortedChildPaths) {
        const childFile = await getVFileSystemApi().findFile(childPath);
        if (!childFile) {
          throw new Error(`File not found: ${childPath}`);
        }
        await buildTree(nodes, childFile, file);
      }
    }
  };

  const loadTree = useCallback(async () => {
    const rootFile = await getVFileSystemApi().getRootDir();
    const nodes: VFileNodeModel[] = [];
    await buildTree(nodes, rootFile);
    setTreeData(nodes);
    console.log('treeData', nodes);
  }, [setTreeData]);

  useEffect(
    withCrash_(async () => {
      await loadTree();
    }),
    [loadTree],
  );

  // reload tree when file is created/changed/deleted
  useEffect(() => {
    const unsubscribe = getEventApi().subscribeToObjectChangeEvent(async (id) => {
      if (id.className === FileClassName) {
        await loadTree();
      }
    });
    return () => {
      unsubscribe();
    };
  }, []);

  const renameFile = useCallback(
    withCrash(async (file: VFile, newName: string) => {
      const pathParts = file.path.split('/');
      const parentPath = pathParts.slice(0, pathParts.length - 1).join('/');
      await getVFileSystemApi().moveFile(file.path, `${parentPath}/${newName}`);
      setFileToRename(undefined);
    }),
    [setFileToRename],
  );

  const findTreeNodeByPath = (path: string): VFileNodeModel | undefined => {
    for (const node of treeData) {
      if (node.data?.path === path) {
        return node;
      }
    }
    return undefined;
  };

  // used to open parent directory when renaming file in it
  useEffect(() => {
    if (fileToRename) {
      const node = findTreeNodeByPath(fileToRename);
      if (node) {
        treeRef.current?.open(node.parent);
      }
    }
  }, [treeData, fileToRename]);

  useEffect(
    withCrash_(async () => {
      await loadTree();
    }),
    [loadTree],
  );

  return (
    <DndProvider backend={MultiBackend} options={getBackendOptions()}>
      <VFileSystemProvider
        value={{
          fileToRename,
          setFileToRename,
          renameFile,
          openFile: onFileOpened,
        }}
      >
        <Tree
          ref={treeRef}
          tree={treeData}
          rootId={0}
          render={(node, params) => <VFileNode node={node} renderParams={params} />}
          onDrop={handleDrop}
          classes={{
            root: classes.root,
            draggingSource: classes.draggingSource,
            dropTarget: classes.dropTarget,
          }}
        />
      </VFileSystemProvider>
    </DndProvider>
  );
}
