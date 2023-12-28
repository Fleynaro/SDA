import { instance_of } from 'sda-bindings';
import { EventPipe } from 'sda-core';
import { Transaction } from 'database';
import { FileChangedEvent, FileCreatedEvent, FileDeletedEvent } from './file/file-events';
import { FILE_CLASS_NAME } from './file';

export const FILESYSTEM_DATABASE_TABLE = FILE_CLASS_NAME;

/**
 * Creates a pipe to synchronize the database with the file system.
 * Allows to store files in the database.
 * @param transaction The transaction object.
 * @returns The created pipe.
 */
export const CreateFileSystemDataBasePipe = (transaction: Transaction) => {
  const pipe = EventPipe.New('filesystem-database');
  pipe.subscribe((event) => {
    if (instance_of(event, FileCreatedEvent)) {
      const e = event as FileCreatedEvent;
      transaction.markAsNew(e.file);
    } else if (instance_of(event, FileChangedEvent)) {
      const e = event as FileChangedEvent;
      transaction.markAsModified(e.file);
    } else if (instance_of(event, FileDeletedEvent)) {
      const e = event as FileDeletedEvent;
      transaction.markAsRemoved(e.file);
    }
  });
  return pipe;
};
