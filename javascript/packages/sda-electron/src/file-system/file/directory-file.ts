import { FileSystem } from '../file-system';
import { AbstractFile, FileDto, FileId, FileType, IFile } from './abstract-file';

export interface DirectoryFileDto extends FileDto {
  readonly childrenIds: FileId[];
}

export interface IDirectoryFile extends IFile {
  readonly children: IFile[];

  find(path: string): IFile | undefined;
}

export class DirectoryFile extends AbstractFile implements IDirectoryFile {
  public children: AbstractFile[] = [];

  constructor(fs: FileSystem, id: FileId, name?: string) {
    super(fs, id, name);
  }

  public get type(): FileType {
    return FileType.Directory;
  }

  public find(path: string): AbstractFile | undefined {
    if (path === '') {
      return this;
    }
    const [first, ...rest] = path.split('/');
    const child = this.children.find((child) => child.name === first);
    if (!child) {
      return undefined;
    }
    if (child instanceof DirectoryFile) {
      return child.find(rest.join('/'));
    }
    if (rest.length > 0) {
      return undefined;
    }
    return child;
  }

  public deserialize(dto: DirectoryFileDto) {
    super.deserialize(dto);
    if (!this.parent) {
      this.fs.deleteFile(this.fs.rootDirectory);
      this.fs.rootDirectory = this;
    }
  }

  public serialize(): DirectoryFileDto {
    return {
      ...super.serialize(),
      type: FileType.Directory,
      childrenIds: this.children.map((child) => child.id),
    };
  }

  public destroy() {
    this.children.forEach((child) => child.destroy());
    super.destroy();
  }
}
