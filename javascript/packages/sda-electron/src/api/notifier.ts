import { ObjectId } from './common';

export enum ObjectChangeType {
    Create,
    Update,
    Delete
}

export interface NotifierController {
    notifyAboutObjectChange(id: ObjectId, changeType: ObjectChangeType): Promise<void>;
}