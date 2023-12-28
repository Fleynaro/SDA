import { DatabaseObjectDto, IDatabaseObject } from 'database';
import { DirectoryFile, IDirectoryFile } from './directory-file';
import { FileSystem } from '../file-system';
import { FileChangedEvent, FileCreatedEvent, FileDeletedEvent } from './file-events';

export const FILE_CLASS_NAME = 'file';

export type FileId = string;

export enum FileType {
  File,
  Directory,
}

export interface FileDto extends DatabaseObjectDto {
  readonly type: FileType;
  readonly name: string;
  readonly fs: string;
  readonly parentId?: FileId;
  readonly createdAt: Date;
}

export interface IFile {
  readonly id: FileId;
  readonly type: FileType;
  name: string;
  readonly path: string;
  parent?: IDirectoryFile;

  serialize(): FileDto;
}

export abstract class AbstractFile implements IDatabaseObject, IFile {
  public readonly id: FileId;
  public readonly fs: FileSystem;
  private _name: string;
  private _parent?: DirectoryFile;
  private _createdAt: Date = new Date();

  constructor(fs: FileSystem, id: FileId, name?: string) {
    this.fs = fs;
    this.id = id;
    this._name = name || '';
    this.fs.pipe.send(new FileCreatedEvent(this));
  }

  public get type(): FileType {
    return FileType.File;
  }

  public get name(): string {
    return this._name;
  }

  public set name(name: string) {
    if (name === this._name) {
      return;
    }
    if (this._parent) {
      this.throwIfExists(this._parent, name);
    }
    this._name = name;
    this.notifyChange();
  }

  public get parent(): DirectoryFile | undefined {
    return this._parent;
  }

  public set parent(parent: DirectoryFile | undefined) {
    if (parent === this._parent) {
      return;
    }
    this.setParent(parent);
  }

  public get createdAt(): Date {
    return this._createdAt;
  }

  public get path(): string {
    if (this.parent) {
      return `${this.parent.path}/${this.name}`;
    }
    return '';
  }

  public serialize(): FileDto {
    return {
      class: FILE_CLASS_NAME,
      uuid: this.id,
      type: this.type,
      fs: this.fs.name,
      name: this.name,
      parentId: this.parent?.id,
      createdAt: this.createdAt,
    };
  }

  public deserialize(dto: FileDto) {
    this._name = dto.name;
    if (dto.parentId) {
      const parent = this.fs.getFileById(dto.parentId);
      if (parent.type === FileType.Directory) {
        this.setParent(parent as DirectoryFile, false);
      } else {
        throw new Error(`Parent is not a directory`);
      }
    }
    this._createdAt = dto.createdAt;
    this.notifyChange();
  }

  public notifyChange() {
    this.fs.pipe.send(new FileChangedEvent(this));
  }

  public destroy() {
    this.parent = undefined;
    this.fs.fileIndex.removeFile(this);
    this.fs.pipe.send(new FileDeletedEvent(this));
  }

  private setParent(parent: DirectoryFile | undefined, notify = true) {
    if (parent) {
      this.throwIfExists(parent, this.name);
    }
    if (this._parent) {
      this._parent.children = this._parent.children.filter((child) => child !== this);
      this._parent.notifyChange();
    }
    this._parent = parent;
    if (notify) {
      this.notifyChange();
    }
    if (parent) {
      parent.children.push(this);
      parent.notifyChange();
    }
  }

  private throwIfExists(directory: DirectoryFile, name: string) {
    if (directory.find(name)) {
      throw new Error(`File ${name} already exists in ${directory.path}`);
    }
  }
}
