declare module sda_core {
    class DataType extends ContextObject {
        size: number;
        isVoid: boolean;

        getType: DataType;
    }

    class VoidDataType extends DataType {
        static create(context: Context): VoidDataType;
    }
}