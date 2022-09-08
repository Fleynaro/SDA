declare module sda {
    abstract class Object implements ISerializable {
        id: string;
        
        setTemporary(temporary: boolean): void;

        serialize(): object;

        deserialize(data: object): void;
    }

    abstract class ContextObject extends Object {
        name: string;
        comment: string;
    }
}