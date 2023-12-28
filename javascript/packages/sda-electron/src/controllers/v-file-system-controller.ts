import app from 'app';
import BaseController from './base-controller';
import { VFile as VFileDto, VFileSystemController } from 'api/v-file-system';
import { toFileDTO } from './dto/v-file-system';
import { FileType, IContentFile, IDirectoryFile } from 'file-system/file';

class VFileSystemControllerImpl extends BaseController implements VFileSystemController {
  constructor() {
    super('VFileSystem');
    this.register('getRootDir', this.getRootDir);
    this.register('findFile', this.findFile);
    this.register('createDirectory', this.createDirectory);
    this.register('createFile', this.createFile);
    this.register('moveFile', this.moveFile);
    this.register('copyFile', this.copyFile);
    this.register('deleteFile', this.deleteFile);
    this.register('readFile', this.readFile);
    this.register('writeFile', this.writeFile);
  }

  public async getRootDir(): Promise<VFileDto> {
    return toFileDTO(this.FS.rootDirectory);
  }

  public async findFile(path: string): Promise<VFileDto | null> {
    const file = this.FS.find(path);
    if (!file) {
      return null;
    }
    return toFileDTO(file);
  }

  public async createDirectory(path: string): Promise<VFileDto> {
    const { name, parentDir } = this.parsePath(path, true);
    if (!name) {
      throw new Error('Name is required');
    }
    const newDirectory = this.FS.newDirectory(parentDir, name);
    return toFileDTO(newDirectory);
  }

  public async createFile(path: string): Promise<VFileDto> {
    const { name, parentDir } = this.parsePath(path, true);
    if (!name) {
      throw new Error('Name is required');
    }
    const newFile = this.FS.newDataTypeDefFile(parentDir, name);
    return toFileDTO(newFile);
  }

  public async moveFile(srcPath: string, destPath: string): Promise<VFileDto> {
    const srcFile = this.FS.find(srcPath);
    if (!srcFile) {
      throw new Error(`File ${srcPath} does not exist`);
    }
    const { name, parentDir } = this.parsePath(destPath, true);
    if (!name) {
      throw new Error('Name is required');
    }
    srcFile.parent = parentDir;
    srcFile.name = name;
    return toFileDTO(srcFile);
  }

  public async copyFile(srcPath: string, destPath: string): Promise<VFileDto> {
    const srcFile = this.FS.find(srcPath);
    if (!srcFile) {
      throw new Error(`File ${srcPath} does not exist`);
    }
    const { name, parentDir } = this.parsePath(destPath, true);
    if (!name) {
      throw new Error('Name is required');
    }
    const destFile = this.FS.cloneFile(srcFile, name);
    destFile.parent = parentDir;
    return toFileDTO(destFile);
  }

  public async deleteFile(path: string): Promise<void> {
    const file = this.FS.find(path);
    if (!file) {
      throw new Error(`File ${path} does not exist`);
    }
    this.FS.deleteFile(file);
  }

  public async readFile(path: string): Promise<string> {
    const file = this.FS.find(path);
    if (!file) {
      throw new Error(`File ${path} does not exist`);
    }
    if (file.type !== FileType.File) {
      throw new Error(`File ${path} is not a file`);
    }
    const contentFile = file as IContentFile;
    return contentFile.content.read();
  }

  public async writeFile(path: string, content: string): Promise<void> {
    const file = this.FS.find(path);
    if (!file) {
      throw new Error(`File ${path} does not exist`);
    }
    if (file.type !== FileType.File) {
      throw new Error(`File ${path} is not a file`);
    }
    const contentFile = file as IContentFile;
    contentFile.content.write(content);
  }

  private parsePath(path: string, throwIfExist = false) {
    const parts = path.split('/');
    const name = parts.pop();
    const parentPath = parts.join('/');
    const file = this.FS.find(parentPath);
    if (!file || file.type !== FileType.Directory) {
      throw new Error(`Directory ${parentPath} does not exist`);
    }
    const parentDir = file as IDirectoryFile;
    if (throwIfExist && name && parentDir.find(name)) {
      throw new Error(`File ${path} already exists`);
    }
    return { name, parentDir };
  }

  private get FS() {
    if (!app.currentProject) {
      throw new Error('No project is open');
    }
    return app.currentProject.fileSystem;
  }
}

export default VFileSystemControllerImpl;
