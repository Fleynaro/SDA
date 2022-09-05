declare module sda_core {
    class DataType extends ContextObject {
        size: number;
        isVoid: boolean;
    }

    class VoidDataType extends DataType {
        static create(context: Context): VoidDataType;
    }
}