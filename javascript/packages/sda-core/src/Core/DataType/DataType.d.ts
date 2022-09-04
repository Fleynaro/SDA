declare module sda_core {
    abstract class Object {
        id: string;
    }

    abstract class ContextObject extends Object {
        name: string;
        comment: string;
    }

    class DataType extends ContextObject {
        size: number;
        isVoid: boolean;
    }

    class VoidDataType extends DataType {
        static create(context: Context): VoidDataType;
    }
}