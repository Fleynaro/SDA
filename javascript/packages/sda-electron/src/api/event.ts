import { ObjectId, ObjectChangeType } from './common';
import { WindowName } from './window';

export enum EventName {
    ObjectChange = "ObjectChange",
    WindowOpen = "WindowOpen"
}

export type WindowOpenEventCallback = (name: WindowName, payload: any) => void;

export type ObjectChangeEventCallback = (id: ObjectId, changeType: ObjectChangeType) => void;

export interface EventController {
    subscribeToWindowOpenEvent(callback: WindowOpenEventCallback): () => void;

    subscribeToObjectChangeEvent(callback: ObjectChangeEventCallback): () => void;
}

export const getEventApi = (window: any) => {
    return window.eventApi as EventController;
}