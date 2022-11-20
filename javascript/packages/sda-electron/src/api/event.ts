import { ObjectId, ObjectChangeType, window_ } from './common';

export enum EventName {
  ObjectChange = 'Event.ObjectChange',
}

export type ObjectChangeEventCallback = (id: ObjectId, changeType: ObjectChangeType) => void;

export interface EventController {
  subscribeToObjectChangeEvent(callback: ObjectChangeEventCallback): () => void;
}

export const getEventApi = () => {
  return window_.eventApi as EventController;
};
