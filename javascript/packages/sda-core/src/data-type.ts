import m from './module';
import { Context } from './context';
import { ContextObject } from './object';
import { FunctionParameterSymbol } from './symbol';
import { SymbolTable } from './symbol-table';
import { CallingConvention } from './platform';

export declare abstract class DataType extends ContextObject {
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

export declare class VoidDataType extends DataType {
  static New(context: Context): VoidDataType;
}

export declare class PointerDataType extends DataType {
  readonly pointedType: DataType;

  static New(context: Context, pointedType: DataType): PointerDataType;
}

export declare class ArrayDataType extends DataType {
  readonly elementType: DataType;
  readonly dimensions: number[];

  static New(context: Context, elementType: DataType, dimensions: number[]): ArrayDataType;
}

export enum ScalarType {
  UnsignedInt,
  SignedInt,
  FloatingPoint,
}

export declare class ScalarDataType extends DataType {
  readonly scalarType: ScalarType;

  static New(context: Context, name: string, type: ScalarType, size: number): ScalarDataType;
}

export declare class TypedefDataType extends DataType {
  readonly refType: DataType;

  static New(context: Context, name: string, refType: DataType): TypedefDataType;
}

export declare class EnumDataType extends DataType {
  fields: { [key: number]: string };

  static New(context: Context, name: string, fields: { [key: number]: string }): EnumDataType;
}

export declare class StructureDataType extends DataType {
  size: number;
  readonly symbolTable: SymbolTable;

  static New(context: Context, name: string, size: number): StructureDataType;
}

export declare class SignatureDataType extends DataType {
  readonly callingConvention: CallingConvention;
  returnType: DataType;
  parameters: FunctionParameterSymbol[];

  static New(
    context: Context,
    callingConvention: CallingConvention,
    name: string,
    returnType: DataType,
    parameters: FunctionParameterSymbol[],
  ): SignatureDataType;
}

module.exports = {
  ...module.exports,
  DataType: m.DataType,
  VoidDataType: m.VoidDataType,
  PointerDataType: m.PointerDataType,
  ArrayDataType: m.ArrayDataType,
  ScalarDataType: m.ScalarDataType,
  TypedefDataType: m.TypedefDataType,
  EnumDataType: m.EnumDataType,
  StructureDataType: m.StructureDataType,
  SignatureDataType: m.SignatureDataType,
};
