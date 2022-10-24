export type ObjectId = {
    key: string;
    className: string;
}

export interface Identifiable {
    id: ObjectId;
}

export const CmpObjectIds = (a: ObjectId, b: ObjectId) =>
    a.key === b.key && a.className === b.className;

export enum ObjectChangeType {
    Create,
    Update,
    Delete
}

export let window_: any = null;

export const setWindow = (win: any) => {
    window_ = win;
}