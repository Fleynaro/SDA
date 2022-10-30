import { Context } from "./context";
import { ContextObject } from "./object";
import { FunctionParameterSymbol } from './symbol.d';
import { SymbolTable } from "./symbol-table";
import { CallingConvention } from "./platform";
import { Hash } from "./utils";

export abstract class DataType extends ContextObject {
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

export class VoidDataType extends DataType {
    static New(context: Context): VoidDataType;
}

export class PointerDataType extends DataType {
    readonly pointedType: DataType;

    static New(context: Context, pointedType: DataType): PointerDataType;
}

export class ArrayDataType extends DataType {
    readonly elementType: DataType;
    readonly dimensions: number[];

    static New(context: Context, elementType: DataType, dimensions: number[]): ArrayDataType;
}

export type ScalarType =
    "UnsignedInt"   |
    "SignedInt"     |
    "FloatingPoint";

export class ScalarDataType extends DataType {
    readonly scalarType: ScalarType;

    static New(context: Context, name: string, type: ScalarType, size: number): ScalarDataType;
}

export class TypedefDataType extends DataType {
    readonly refType: DataType;

    static New(context: Context, name: string, refType: DataType): TypedefDataType;
}

export class EnumDataType extends DataType {
    fields: { [key: number]: string };

    static New(context: Context, name: string, fields: { [key: number]: string }): EnumDataType;
}

export class StructureDataType extends DataType {
    size: number;
    readonly symbolTable: SymbolTable;

    static New(context: Context, name: string, size: number): StructureDataType;
}

export class SignatureDataType extends DataType {
    readonly callingConvention: CallingConvention;
    returnType: DataType;
    parameters: FunctionParameterSymbol[];

    static New(
        context: Context,
        callingConvention: CallingConvention,
        name: string, returnType: DataType,
        parameters: FunctionParameterSymbol[]): SignatureDataType;
}