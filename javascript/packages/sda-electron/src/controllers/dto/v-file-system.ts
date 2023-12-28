import { VFile as VFileDTO } from 'api/v-file-system';
import { FileType, IDirectoryFile, IFile } from 'file-system/file';
import { StringToHash } from 'sda-core';

export const toFileDTO = (file: IFile): VFileDTO => {
  if (file.type === FileType.File) {
    return {
      type: 'file',
      id: Number(StringToHash(file.id)),
      name: file.name,
      path: file.path,
    };
  }
  const dir = file as IDirectoryFile;
  return {
    type: 'directory',
    id: Number(StringToHash(file.id)),
    name: file.name,
    path: file.path,
    files: dir.children.map((child) => child.path),
  };
};
