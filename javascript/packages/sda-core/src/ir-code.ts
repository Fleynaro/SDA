import m from './module';
import { EventPipe } from './event';
import {
  InstructionOffset,
  PcodeBlock,
  PcodeConstantVarnode,
  PcodeFunctionGraph,
  PcodeGraph,
  PcodeInstruction,
  PcodePrinter,
  PcodeRegisterVarnode,
  PcodeStructTreePrinter,
} from './p-code';
import { CallingConvention } from './platform';
import { SymbolTable } from './symbol-table';
import { FunctionSymbol } from './symbol';
import { AbstractPrinter, AbstractPrinterToken, Hash, IIdentifiable } from './utils';

export enum IRcodeValueType {
  Constant,
  Register,
  Variable,
}

export declare abstract class IRcodeValue {
  readonly type: IRcodeValueType;
  readonly size: number;
  readonly hash: Hash;
}

export declare class IRcodeConstant extends IRcodeValue {
  readonly constVarnode: PcodeConstantVarnode;
}

export declare class IRcodeRegister extends IRcodeValue {
  readonly regVarnode: PcodeRegisterVarnode;
}

export declare class IRcodeVariable extends IRcodeValue {
  readonly name: string;
  readonly fullName: string;
  readonly isUsed: boolean;
  readonly sourceOperation: IRcodeOperation;
}

export enum IRcodeOperationId {
  NONE,
  UNKNOWN,

  // Data Moving
  COPY,
  LOAD,
  REF,

  // Arithmetic
  INT_ADD,
  INT_SUB,
  INT_CARRY,
  INT_SCARRY,
  INT_SBORROW,
  INT_2COMP,
  INT_MULT,
  INT_DIV,
  INT_SDIV,
  INT_REM,
  INT_SREM,

  // Logical
  INT_NEGATE,
  INT_XOR,
  INT_AND,
  INT_OR,
  INT_LEFT,
  INT_RIGHT,
  INT_SRIGHT,

  // Integer Comparison
  INT_EQUAL,
  INT_NOTEQUAL,
  INT_SLESS,
  INT_SLESSEQUAL,
  INT_LESS,
  INT_LESSEQUAL,

  // Boolean
  BOOL_NEGATE,
  BOOL_XOR,
  BOOL_AND,
  BOOL_OR,

  // Floating point
  FLOAT_ADD,
  FLOAT_SUB,
  FLOAT_MULT,
  FLOAT_DIV,
  FLOAT_NEG,
  FLOAT_ABS,
  FLOAT_SQRT,

  // Floating point compare
  FLOAT_NAN,
  FLOAT_EQUAL,
  FLOAT_NOTEQUAL,
  FLOAT_LESS,
  FLOAT_LESSEQUAL,

  // Other
  PHI,
  EXTRACT,
  CONCAT,
  CALL,
}

export declare abstract class IRcodeOperation {
  readonly id: IRcodeOperationId;
  readonly hash: Hash;
  readonly output: IRcodeVariable;
  readonly block: IRcodeBlock;
  readonly pcodeInstruction: PcodeInstruction;
}

export declare class IRcodeUnaryOperation extends IRcodeOperation {
  readonly input: IRcodeValue;
}

export declare class IRcodeBinaryOperation extends IRcodeOperation {
  readonly input1: IRcodeValue;
  readonly input2: IRcodeValue;
}

export declare class IRcodeBlock {
  readonly name: string;
  readonly hash: Hash;
  readonly pcodeBlock: PcodeBlock;
  readonly function: IRcodeFunction;
  readonly nearNextBlock: IRcodeBlock;
  readonly farNextBlock: IRcodeBlock;
  readonly referencedBlocks: IRcodeBlock[];
  readonly operations: IRcodeOperation[];
}

export declare class IRcodeFunction {
  readonly name: string;
  readonly entryBlock: IRcodeBlock;
  readonly entryOffset: InstructionOffset;
  readonly program: IRcodeProgram;
  readonly funcGraph: PcodeFunctionGraph;
  readonly funcSymbol: FunctionSymbol;
  readonly variables: IRcodeVariable[];
  readonly paramVariables: IRcodeVariable[];
  readonly returnVariable: IRcodeVariable;

  findVariableById(id: number): IRcodeVariable;
}

export declare class IRcodeProgram implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly eventPipe: EventPipe;
  readonly graph: PcodeGraph;
  readonly globalSymbolTable: SymbolTable;

  getFunctionAt(offset: InstructionOffset): IRcodeFunction;

  getFunctionsByCallInstruction(instr: PcodeInstruction): IRcodeFunction[];

  getCallsRefToFunction(func: IRcodeFunction): IRcodeOperation[];

  static New(graph: PcodeGraph, globalSymbolTable: SymbolTable): IRcodeProgram;

  static Get(hashId: Hash): IRcodeProgram;
}

export declare class IRcodeContextSync {
  eventPipe: EventPipe;

  static New(
    program: IRcodeProgram,
    globalSymbolTable: SymbolTable,
    callingConvention: CallingConvention,
  ): IRcodeContextSync;
}

export declare class IRcodePcodeSync {
  eventPipe: EventPipe;

  static New(program: IRcodeProgram): IRcodePcodeSync;
}

export enum IRcodePrinterToken {
  Operation = AbstractPrinterToken.Keyword,
  Variable = AbstractPrinterToken.Identifier,
}

export declare class IRcodePrinter extends AbstractPrinter {
  combineWithStructPrinter(structPrinter: PcodeStructTreePrinter, func: IRcodeFunction): void;

  printOperation(operation: IRcodeOperation): void;

  printValue(value: IRcodeValue, extended: boolean): void;
}

export declare class IRcodePrinterJs extends IRcodePrinter {
  printOperationImpl: (operation: IRcodeOperation) => void;

  printValueImpl: (value: IRcodeValue, extended: boolean) => void;

  static New(printer: PcodePrinter): IRcodePrinterJs;
}

module.exports = {
  ...module.exports,
  IRcodeValue: m.IRcodeValue,
  IRcodeConstant: m.IRcodeConstant,
  IRcodeRegister: m.IRcodeRegister,
  IRcodeVariable: m.IRcodeVariable,
  IRcodeOperation: m.IRcodeOperation,
  IRcodeUnaryOperation: m.IRcodeUnaryOperation,
  IRcodeBinaryOperation: m.IRcodeBinaryOperation,
  IRcodeBlock: m.IRcodeBlock,
  IRcodeFunction: m.IRcodeFunction,
  IRcodeProgram: m.IRcodeProgram,
  IRcodeContextSync: m.IRcodeContextSync,
  IRcodePcodeSync: m.IRcodePcodeSync,
  IRcodePrinter: m.IRcodePrinter,
  IRcodePrinterJs: m.IRcodePrinterJs,
};
