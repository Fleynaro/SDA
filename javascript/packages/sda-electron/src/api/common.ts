export type ObjectId = {
    key: string;
    className: string;
}

export interface Identifiable {
    id: ObjectId;
}