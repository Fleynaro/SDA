import m from './module';
import { EventPipe } from './event';
import { Platform, RegisterRepository } from './platform';
import { Offset, AbstractPrinter, AbstractPrinterToken } from './utils';
import { Image } from './image';

export declare abstract class PcodeVarnode {
  readonly size: number;
  readonly isRegister: boolean;
}

export type InstructionOffset = number;

export const toInstructionOffset = (byteOffset: Offset, index = 0): InstructionOffset =>
  (byteOffset << 8) | index;

export enum PcodeInstructionId {
  NONE,
  UNKNOWN,
  NOP,

  // Data Moving
  COPY,
  LOAD,
  STORE,

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

  // Floating point conversion
  INT2FLOAT,
  FLOAT2INT,
  FLOAT2FLOAT,
  TRUNC,
  FLOAT_CEIL,
  FLOAT_FLOOR,
  FLOAT_ROUND,

  // Branching
  BRANCH,
  CBRANCH,
  BRANCHIND,
  CALL,
  CALLIND,
  RETURN,

  // Extension / truncation
  INT_ZEXT,
  INT_SEXT,
  PIECE,
  SUBPIECE,

  // Managed Code
  CPOOLREF,
  NEW,

  // Interruption
  INT,
}

export declare class PcodeInstruction {
  readonly id: PcodeInstructionId;
  readonly input0: PcodeVarnode;
  readonly input1: PcodeVarnode;
  readonly output: PcodeVarnode;
  readonly offset: InstructionOffset;
}

export declare class PcodeBlock {
  readonly name: string;
  readonly index: number;
  readonly graph: PcodeGraph;
  readonly minOffset: InstructionOffset;
  readonly maxOffset: InstructionOffset;
  readonly nearNextBlock: PcodeBlock;
  readonly farNextBlock: PcodeBlock;
  readonly referencedBlocks: PcodeBlock[];
  readonly instructions: { [offset: InstructionOffset]: PcodeInstruction };
  readonly lastInstruction: PcodeInstruction;

  contains(offset: InstructionOffset, halfInterval: boolean): boolean;
}

export declare class PcodeFunctionGraph {
  readonly name: string;
  readonly entryBlock: PcodeBlock;
  readonly entryOffset: InstructionOffset;
  readonly graph: PcodeGraph;
  readonly referencedGraphsTo: PcodeFunctionGraph[];
  readonly referencedGraphsFrom: PcodeFunctionGraph[];

  addReferencedGraphFrom(fromOffset: InstructionOffset, referencedGraph: PcodeFunctionGraph): void;

  removeReferencedGraphFrom(fromOffset: InstructionOffset): void;
}

export declare class PcodeGraph {
  exploreImage(startOffset: InstructionOffset, image: Image): void;

  exploreInstructions(startOffset: InstructionOffset, instructions: PcodeInstruction[]): void;

  addInstruction(instruction: PcodeInstruction): void;

  removeInstruction(instruction: PcodeInstruction): void;

  getInstructionAt(offset: InstructionOffset): PcodeInstruction;

  getInstructionsAt(byteOffset: Offset): PcodeInstruction[];

  createBlock(offset: InstructionOffset): PcodeBlock;

  removeBlock(block: PcodeBlock): void;

  getBlockAt(offset: InstructionOffset, halfInterval: boolean): PcodeBlock;

  createFunctionGraph(entryBlock: PcodeBlock): PcodeFunctionGraph;

  removeFunctionGraph(graph: PcodeFunctionGraph): void;

  getFunctionGraphAt(offset: InstructionOffset): PcodeFunctionGraph;

  static New(eventPipe: EventPipe, platform: Platform): PcodeGraph;
}

export declare class PcodeParser {
  static Parse(text: string, regRepo: RegisterRepository | null): PcodeInstruction[];
}

export enum PcodePrinterToken {
  Mneumonic = AbstractPrinterToken.Keyword,
  Register = AbstractPrinterToken.Identifier,
  VirtRegister = AbstractPrinterToken.Identifier,
}

export declare class PcodePrinter extends AbstractPrinter {
  printInstruction(pcode: PcodeInstruction): void;

  printInstructionImpl: (pcode: PcodeInstruction) => void;

  printVarnode(varnode: PcodeVarnode, printSizeAndOffset: boolean): void;

  printVarnodeImpl: (varnode: PcodeVarnode, printSizeAndOffset: boolean) => void;

  static New(regRepo: RegisterRepository | null): PcodePrinter;

  static Print(instruction: PcodeInstruction, regRepo: RegisterRepository | null): string;
}

module.exports = {
  ...module.exports,
  PcodeVarnode: m.PcodeVarnode,
  PcodeInstruction: m.PcodeInstruction,
  PcodeBlock: m.PcodeBlock,
  PcodeFunctionGraph: m.PcodeFunctionGraph,
  PcodeGraph: m.PcodeGraph,
  PcodeParser: m.PcodeParser,
  PcodePrinter: m.PcodePrinter,
};
