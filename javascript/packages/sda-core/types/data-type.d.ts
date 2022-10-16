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

    static Get(hashId: Hash): VoidDataType;
}

export class PointerDataType extends DataType {
    readonly pointedType: DataType;

    static New(context: Context, pointedType: DataType): PointerDataType;

    static Get(hashId: Hash): PointerDataType;
}

export class ArrayDataType extends DataType {
    readonly elementType: DataType;
    readonly dimensions: number[];

    static New(context: Context, elementType: DataType, dimensions: number[]): ArrayDataType;

    static Get(hashId: Hash): ArrayDataType;
}

export type ScalarType =
    "UnsignedInt"   |
    "SignedInt"     |
    "FloatingPoint";

export class ScalarDataType extends DataType {
    readonly scalarType: ScalarType;

    static New(context: Context, name: string, type: ScalarType, size: number): ScalarDataType;

    static Get(hashId: Hash): ScalarDataType;
}

export class TypedefDataType extends DataType {
    readonly refType: DataType;

    static New(context: Context, name: string, refType: DataType): TypedefDataType;

    static Get(hashId: Hash): TypedefDataType;
}

export class EnumDataType extends DataType {
    fields: { [key: number]: string };

    static New(context: Context, name: string, fields: { [key: number]: string }): EnumDataType;

    static Get(hashId: Hash): EnumDataType;
}

export class StructureDataType extends DataType {
    size: number;
    readonly symbolTable: SymbolTable;

    static New(context: Context, name: string, size: number): StructureDataType;

    static Get(hashId: Hash): StructureDataType;
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

    static Get(hashId: Hash): SignatureDataType;
}