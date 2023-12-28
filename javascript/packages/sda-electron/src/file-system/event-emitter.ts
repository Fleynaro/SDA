import { instance_of } from 'sda-bindings';
import { EventPipe } from 'sda-core';
import { FileChangedEvent, FileCreatedEvent, FileDeletedEvent } from './file/file-events';
import { ObjectChangeType, objectChangeEmitter } from 'eventEmitter';
import { FileClassName } from 'api/v-file-system';

export const CreateGuiEventEmitterPipe = () => {
  const pipe = EventPipe.New('gui-event-emitter');
  pipe.subscribe((event) => {
    if (instance_of(event, FileCreatedEvent)) {
      const e = event as FileCreatedEvent;
      objectChangeEmitter()({ className: FileClassName, key: e.file.id }, ObjectChangeType.Create);
    } else if (instance_of(event, FileChangedEvent)) {
      const e = event as FileChangedEvent;
      objectChangeEmitter()({ className: FileClassName, key: e.file.id }, ObjectChangeType.Update);
    } else if (instance_of(event, FileDeletedEvent)) {
      const e = event as FileDeletedEvent;
      objectChangeEmitter()({ className: FileClassName, key: e.file.id }, ObjectChangeType.Delete);
    }
  });
  return pipe;
};
