import { EventName, ObjectChangeEventCallback } from 'api/event';
import { sendMessageToAllWindows } from 'utils/window';

export { ObjectChangeType } from 'api/common';

export const objectChangeEmitter = () =>
  function (id, changeType) {
    sendMessageToAllWindows(EventName.ObjectChange, id, changeType);
  } as ObjectChangeEventCallback;
