import assert from 'assert';
import { randomUUID } from 'crypto';
import { AbstractFile, FileDto, FileId, FileType, IFile } from './file/abstract-file';
import { DirectoryFile, IDirectoryFile } from './file/directory-file';
import { ContentFile, IContentFile } from './file/content-file';
import { AbstractContent } from './content/abstract-content';
import { EventPipe } from 'sda-core';
import { FileIndex } from './file-index';
import { Project } from 'project';
import { DataTypeContent } from './content/data-type-content';

export class FileSystem {
  public readonly name: string;
  private readonly project: Project;
  public pipe: EventPipe;
  public rootDirectory: IDirectoryFile;
  public fileIndex: FileIndex = new FileIndex();

  constructor(name: string, project: Project, pipe: EventPipe) {
    this.name = name;
    this.project = project;
    this.pipe = pipe;
    this.rootDirectory = new DirectoryFile(this, randomUUID(), '');
    this.addFileToIndex(this.rootDirectory as DirectoryFile);
  }

  public getFileById(id: FileId): IFile {
    const file = this.fileIndex.getFilesByKey(id);
    if (file.length === 0) {
      throw new Error(`File with id ${id} not found`);
    }
    assert(file.length === 1, `Multiple files with id ${id} found`);
    return file[0];
  }

  public find(path: string): IFile | undefined {
    if (path.startsWith('/')) {
      path = path.slice(1);
    }
    return this.rootDirectory.find(path);
  }

  public newDirectory(parentDirectory: IDirectoryFile, name: string): IDirectoryFile {
    const directory = new DirectoryFile(this, randomUUID(), name);
    directory.parent = parentDirectory as DirectoryFile;
    this.addFileToIndex(directory);
    this.fileIndex.addFileToKey(directory, directory.id);
    return directory;
  }

  public newFile(
    parentDirectory: IDirectoryFile,
    name: string,
    content: AbstractContent,
  ): IContentFile {
    const file = new ContentFile(this, randomUUID(), content, name);
    file.parent = parentDirectory as DirectoryFile;
    this.addFileToIndex(file);
    return file;
  }

  public newDataTypeDefFile(parentDirectory: IDirectoryFile, name: string): IContentFile {
    return this.newFile(parentDirectory, name, new DataTypeContent(this.project.context));
  }

  public cloneFile(file: IFile, newName: string): IFile {
    const data = {
      ...file.serialize(),
      uuid: randomUUID(),
      name: newName,
    };
    const fileObject = this.createFileObject({
      ...data,
      uuid: randomUUID(),
    });
    fileObject.deserialize(data);
    return fileObject;
  }

  public createFileObject(dto: FileDto): AbstractFile {
    if (dto.type === FileType.File) {
      // const fileDto = dto as ContentFileDto;
      const file = new ContentFile(this, dto.uuid, new DataTypeContent(this.project.context));
      this.addFileToIndex(file);
      return file;
    } else if (dto.type === FileType.Directory) {
      const directory = new DirectoryFile(this, dto.uuid);
      this.addFileToIndex(directory);
      return directory;
    }
    throw new Error(`Unknown file type ${dto.type}`);
  }

  public deleteFile(file: IFile) {
    (file as AbstractFile).destroy();
  }

  private addFileToIndex(file: AbstractFile) {
    this.fileIndex.addFileToKey(file, file.id);
  }
}
