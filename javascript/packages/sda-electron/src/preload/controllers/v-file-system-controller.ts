import { FilePath, VFileSystemController } from 'api/v-file-system';
import { invokerFactory } from '../utils';

const invoke = invokerFactory('VFileSystem');

const VFileSystemControllerImpl: VFileSystemController = {
  getRootDir: () => invoke('getRootDir'),

  findFile: (path: FilePath) => invoke('findFile', path),

  createDirectory: (path: FilePath) => invoke('createDirectory', path),

  createFile: (path: FilePath) => invoke('createFile', path),

  moveFile: (srcPath: FilePath, destPath: FilePath) => invoke('moveFile', srcPath, destPath),

  copyFile: (srcPath: FilePath, destPath: FilePath) => invoke('copyFile', srcPath, destPath),

  deleteFile: (path: FilePath) => invoke('deleteFile', path),

  readFile: (path: FilePath) => invoke('readFile', path),

  writeFile: (path: FilePath, content: string) => invoke('writeFile', path, content),
};

export default VFileSystemControllerImpl;
