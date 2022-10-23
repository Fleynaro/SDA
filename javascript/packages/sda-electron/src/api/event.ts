import { ObjectId, ObjectChangeType } from './common';

export enum EventName {
    ObjectChange = "Event.ObjectChange"
}

export type ObjectChangeEventCallback = (id: ObjectId, changeType: ObjectChangeType) => void;

export interface EventController {
    subscribeToObjectChangeEvent(callback: ObjectChangeEventCallback): () => void;
}

export const getEventApi = (window: any) => {
    return window.eventApi as EventController;
}