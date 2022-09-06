declare module sda_core {
    abstract class DataType extends ContextObject {
        readonly baseType: DataType;
        readonly isVoid: boolean;
        readonly isPointer: boolean;
        readonly isFloatingPoint: boolean;
        readonly isSigned: boolean;
        readonly isUnsigned: boolean;
        readonly size: number;

        getPointerTo(): DataType;

        getArrayOf(dimensions: number[]): DataType;
    }

    class VoidDataType extends DataType {
        static New(context: Context): VoidDataType;
    }

    class PointerDataType extends DataType {
        readonly pointedType: DataType;

        static New(context: Context, pointedType: DataType): PointerDataType;
    }

    class ArrayDataType extends DataType {
        readonly elementType: DataType;
        readonly dimensions: number[];

        static New(context: Context, elementType: DataType, dimensions: number[]): ArrayDataType;
    }

    enum ScalarType {
        UnsignedInt,
        SignedInt,
        FloatingPoint
    }

    class ScalarDataType extends DataType {
        readonly scalarType: ScalarType;

        static New(context: Context, name: string, type: ScalarType, size: number): ScalarDataType;
    }

    class TypedefDataType extends DataType {
        readonly refType: DataType;

        static New(context: Context, name: string, refType: DataType): TypedefDataType;
    }

    class EnumDataType extends DataType {
        fields: { [key: number]: string };

        static New(context: Context, name: string, fields: { [key: number]: string }): EnumDataType;
    }

    class StructureDataType extends DataType {
        size: number;
        readonly symbolTable: SymbolTable;

        static New(context: Context, name: string, size: number): StructureDataType;
    }
}