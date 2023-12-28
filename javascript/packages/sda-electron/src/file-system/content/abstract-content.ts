import { AbstractFile } from 'file-system/file';

// eslint-disable-next-line @typescript-eslint/no-empty-interface
export interface ContentDto {}

export abstract class AbstractContent {
  private _file?: AbstractFile;

  public get file(): AbstractFile {
    if (!this._file) {
      throw new Error('File not set');
    }
    return this._file;
  }

  public set file(file: AbstractFile) {
    this._file = file;
  }

  public abstract read(): string;

  public abstract write(content: string);

  public deserialize(dto: ContentDto) {
    // do nothing
  }

  public serialize(): ContentDto {
    return {};
  }
}
