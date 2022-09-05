declare module sda_core {
    abstract class Object {
        id: string;
        setTemporary(temporary: boolean): void;
    }

    abstract class ContextObject extends Object {
        name: string;
        comment: string;
    }
}