import { ObjectId } from './common';

export { ObjectId };

export enum ObjectChangeType {
    Create,
    Update,
    Delete
}

export interface NotifierController {
    notifyAboutObjectChange(id: ObjectId, changeType: ObjectChangeType): Promise<void>;

    subscribeToObjectChanges(callback: (id: ObjectId, changeType: ObjectChangeType) => void): () => void;
}

export const getNotifierApi = (window: any) => {
    return window.notifierApi as NotifierController;
}