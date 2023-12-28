import { JsEvent } from 'sda-core';
import { IFile } from './abstract-file';

export abstract class FileEvent extends JsEvent {
  readonly file: IFile;
  constructor(file: IFile) {
    super('FileSystem');
    this.file = file;
  }
}

export class FileCreatedEvent extends FileEvent {
  constructor(file: IFile) {
    super(file);
  }
}

export class FileChangedEvent extends FileEvent {
  constructor(file: IFile) {
    super(file);
  }
}

export class FileDeletedEvent extends FileEvent {
  constructor(file: IFile) {
    super(file);
  }
}
