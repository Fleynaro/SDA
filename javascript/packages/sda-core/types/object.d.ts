import { ISerializable } from "./utils";

export abstract class Object implements ISerializable {
    id: string;
    
    setTemporary(temporary: boolean): void;

    serialize(): object;

    deserialize(data: object): void;
}

export abstract class ContextObject extends Object {
    name: string;
    comment: string;
}