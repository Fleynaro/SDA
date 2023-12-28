import { useCallback, useMemo } from 'react';
import { getVFileSystemApi } from 'sda-electron/api/v-file-system';
import { MenuNode } from 'components/Menu';
import { withCrash } from 'providers/CrashProvider';
import { VFileNodeModel } from './VFileNode';
import { useVFileSystem } from './VFileContext';

export interface VFileContextMenuProps {
  node: VFileNodeModel;
}

export const VFileContextMenu = ({ node }: VFileContextMenuProps) => {
  const fileSystem = useVFileSystem();
  const file = node.data!;
  const isDirectory = file.type === 'directory';

  const findFreeFilePath = async (path: string): Promise<string> => {
    for (let i = 1; i < 100; i++) {
      const freePath = `${path}/file-${i}`;
      const foundFile = await getVFileSystemApi().findFile(`${freePath}`);
      if (!foundFile) {
        return freePath;
      }
    }
    throw new Error('Cannot find free file path');
  };

  const onCreateFile = useCallback(
    withCrash(async () => {
      const newFile = await getVFileSystemApi().createFile(await findFreeFilePath(file.path));
      fileSystem.setFileToRename(newFile.path);
    }),
    [file, fileSystem],
  );

  const onCreateDirectory = useCallback(
    withCrash(async () => {
      const newFile = await getVFileSystemApi().createDirectory(await findFreeFilePath(file.path));
      fileSystem.setFileToRename(newFile.path);
    }),
    [file, fileSystem],
  );

  const onRenameFile = useCallback(
    withCrash(async () => {
      fileSystem.setFileToRename(file.path);
    }),
    [file],
  );

  const onDeleteFile = useCallback(
    withCrash(async () => {
      await getVFileSystemApi().deleteFile(file.path);
    }),
    [file],
  );

  return (
    <>
      {isDirectory ? (
        <>
          <MenuNode label="New file" onClick={onCreateFile} />
          <MenuNode label="New directory" onClick={onCreateDirectory} />
        </>
      ) : (
        fileSystem.openFile && <MenuNode label="Open" onClick={() => fileSystem.openFile?.(file)} />
      )}
      {file.path !== '' && (
        <>
          <MenuNode label="Delete" onClick={onDeleteFile} />
          <MenuNode label="Rename" onClick={onRenameFile} />
        </>
      )}
    </>
  );
};
