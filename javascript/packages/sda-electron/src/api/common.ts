export type ObjectId = {
    key: string;
    className: string;
}

export enum ObjectChangeType {
    Create,
    Update,
    Delete
}

export interface Identifiable {
    id: ObjectId;
}