declare module sda_core {
    class DataType {
        size: number;
        isVoid: boolean;
    }

    class VoidDataType extends DataType {
        constructor(ctx: Context);

        test(value: (n: number) => boolean): { a: number, result: boolean };
    }
}