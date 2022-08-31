declare module 'core-js' {
    export class VoidDataType extends DataType {
        constructor(ctx: Context);

        test(value: (n: number) => boolean): { a: number, result: boolean };
    }
}