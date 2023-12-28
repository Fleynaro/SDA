import { FileSystem } from '../file-system';
import { AbstractContent, ContentDto } from '../content/abstract-content';
import { AbstractFile, FileDto, FileId, FileType, IFile } from './abstract-file';

export interface ContentFileDto extends FileDto {
  readonly content: ContentDto;
}

export interface IContentFile extends IFile {
  readonly content: AbstractContent;
}

export class ContentFile extends AbstractFile implements IContentFile {
  public readonly content: AbstractContent;

  constructor(fs: FileSystem, id: FileId, content: AbstractContent, name?: string) {
    super(fs, id, name);
    this.content = content;
    content.file = this;
  }

  public get type(): FileType {
    return FileType.File;
  }

  public deserialize(dto: ContentFileDto) {
    this.content.deserialize(dto.content);
    super.deserialize(dto);
  }

  public serialize(): ContentFileDto {
    return {
      ...super.serialize(),
      type: FileType.File,
      content: this.content.serialize(),
    };
  }
}
