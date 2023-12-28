import { IFile } from './file/abstract-file';

/**
 * Represents a file index that maps keys to an array of files.
 * Allows for quick retrieval of files by key.
 */
export class FileIndex {
  private keyToFiles: Map<string, IFile[]> = new Map();

  /**
   * Retrieves the files associated with the specified key.
   * @param key - The key to retrieve files for.
   * @returns An array of files associated with the key.
   */
  public getFilesByKey(key: string): IFile[] {
    return this.keyToFiles.get(key) || [];
  }

  /**
   * Adds a file to the specified key.
   * @param file - The file to add.
   * @param key - The key to add the file to.
   */
  public addFileToKey(file: IFile, key: string) {
    const files = this.getFilesByKey(key);
    files.push(file);
    this.keyToFiles.set(key, files);
  }

  /**
   * Removes a file from the specified key.
   * @param file - The file to remove.
   * @param key - The key to remove the file from.
   */
  public removeFileFromKey(file: IFile, key: string) {
    const files = this.keyToFiles.get(key);
    if (files) {
      const index = files.indexOf(file);
      if (index >= 0) {
        files.splice(index, 1);
      }
    }
  }

  /**
   * Removes a file from all keys.
   * @param file - The file to remove.
   */
  public removeFile(file: IFile) {
    this.keyToFiles.forEach((files) => {
      const index = files.indexOf(file);
      if (index >= 0) {
        files.splice(index, 1);
      }
    });
  }
}
