import { instance_of } from 'sda-bindings';
import { EventPipe, ObjectAddedEvent, ObjectModifiedEvent, ObjectRemovedEvent } from 'sda-core';
import { IDatabaseObject, Transaction } from 'database';

export const SDA_DATABASE_TABLES = [
  'address_space',
  'image',
  'data_type',
  'symbol',
  'symbol_table',
];

export const CreateSdaDataBasePipe = (transaction: Transaction) => {
  const pipe = EventPipe.New('sda-database');
  pipe.subscribe((event) => {
    if (instance_of(event, ObjectAddedEvent)) {
      const e = event as ObjectAddedEvent;
      transaction.markAsNew(e.object as IDatabaseObject);
    } else if (instance_of(event, ObjectModifiedEvent)) {
      const e = event as ObjectModifiedEvent;
      transaction.markAsModified(e.object as IDatabaseObject);
    } else if (instance_of(event, ObjectRemovedEvent)) {
      const e = event as ObjectRemovedEvent;
      transaction.markAsRemoved(e.object as IDatabaseObject);
    }
  });
  return pipe;
};
