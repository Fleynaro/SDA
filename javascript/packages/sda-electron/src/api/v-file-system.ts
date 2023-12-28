import { window_ } from './common';

export const FileClassName = 'File';

export type FilePath = string;

export type VFile = {
  id: number;
  name: string;
  path: FilePath;
} & (
  | {
      type: 'file';
    }
  | {
      type: 'directory';
      files: FilePath[];
    }
);

export interface VFileSystemController {
  getRootDir(): Promise<VFile>;

  findFile(path: FilePath): Promise<VFile | null>;

  createDirectory(path: FilePath): Promise<VFile>;

  createFile(path: FilePath): Promise<VFile>;

  moveFile(srcPath: FilePath, destPath: FilePath): Promise<VFile>;

  copyFile(srcPath: FilePath, destPath: FilePath): Promise<VFile>;

  deleteFile(path: FilePath): Promise<void>;

  readFile(path: FilePath): Promise<string>;

  writeFile(path: FilePath, content: string): Promise<void>;
}

export const getVFileSystemApi = () => {
  return window_.vFileSystemApi as VFileSystemController;
};
