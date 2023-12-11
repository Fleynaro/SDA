import m from './module';
import { EventPipe } from './event';
import { Platform, RegisterRepository } from './platform';
import { Offset, AbstractPrinter, AbstractPrinterToken, Hash, IIdentifiable } from './utils';
import { Image } from './image';

export declare abstract class PcodeVarnode {
  readonly size: number;
  readonly isRegister: boolean;
}

export declare class PcodeConstantVarnode extends PcodeVarnode {
  readonly value: number;
}

export declare class PcodeRegisterVarnode extends PcodeVarnode {}

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
  readonly functionGraph: PcodeFunctionGraph;
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

export declare class PcodeGraph implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly platform: Platform;

  exploreImage(startOffset: InstructionOffset, image: Image): void;

  exploreInstructions(startOffset: InstructionOffset, instructions: PcodeInstruction[]): void;

  addInstruction(instruction: PcodeInstruction): void;

  removeInstruction(instruction: PcodeInstruction): void;

  getInstructionAt(offset: InstructionOffset): PcodeInstruction | undefined;

  getInstructionsAt(byteOffset: Offset): PcodeInstruction[];

  createBlock(offset: InstructionOffset): PcodeBlock;

  removeBlock(block: PcodeBlock): void;

  getBlockAt(offset: InstructionOffset, halfInterval: boolean): PcodeBlock | undefined;

  createFunctionGraph(entryBlock: PcodeBlock): PcodeFunctionGraph;

  removeFunctionGraph(graph: PcodeFunctionGraph): void;

  getFunctionGraphAt(offset: InstructionOffset): PcodeFunctionGraph | undefined;

  static New(eventPipe: EventPipe, platform: Platform): PcodeGraph;

  static Get(hashId: Hash): PcodeGraph;
}

export enum GotoType {
  Default,
  Continue,
  Break,
}

export declare class PcodeStructBlock {
  readonly name: string;
  readonly pcodeBlock: PcodeBlock;
  readonly nearNextBlock: PcodeStructBlock;
  readonly farNextBlock: PcodeStructBlock;
  readonly referencedBlocks: PcodeStructBlock[];
  readonly goto: PcodeStructBlock;
  readonly gotoType: GotoType;
  readonly gotoRefBlocks: PcodeStructBlock[];
  readonly parent: PcodeStructBlock;
  readonly top: PcodeStructBlock;
  readonly bottom: PcodeStructBlock;
  readonly leafs: PcodeStructBlock[];
}

export declare class PcodeStructBlockSequence extends PcodeStructBlock {
  readonly blocks: PcodeStructBlock[];
}

export declare class PcodeStructBlockIf extends PcodeStructBlock {
  readonly conditionBlock: PcodeStructBlock;
  readonly thenBlock: PcodeStructBlock;
  readonly elseBlock: PcodeStructBlock;
  readonly inverted: boolean;
}

export declare class PcodeStructBlockWhile extends PcodeStructBlock {
  readonly bodyBlock: PcodeStructBlock;
}

export declare class PcodeStructTree {
  readonly entryBlock: PcodeStructBlock;

  init(funcGraph: PcodeFunctionGraph): void;

  static New(): PcodeStructTree;
}

export declare class PcodeStructTreePrinter extends AbstractPrinter {
  printStructBlock(block: PcodeStructBlock): void;

  printStructTree(tree: PcodeStructTree): void;
}

export declare class PcodeStructTreePrinterJs extends PcodeStructTreePrinter {
  printStructBlockImpl: (block: PcodeStructBlock) => void;

  static New(): PcodeStructTreePrinterJs;
}

export declare class PcodeParser {
  static Parse(text: string, regRepo: RegisterRepository | null): PcodeInstruction[];
}

export enum PcodePrinterToken {
  Mneumonic = AbstractPrinterToken.Parent,
  Register = AbstractPrinterToken.Parent + 1,
  VirtRegister = AbstractPrinterToken.Parent + 2,
}

export declare class PcodePrinter extends AbstractPrinter {
  combineWithStructPrinter(structPrinter: PcodeStructTreePrinter): void;

  printInstruction(pcode: PcodeInstruction): void;

  printVarnode(varnode: PcodeVarnode, printSizeAndOffset: boolean): void;

  static Print(instruction: PcodeInstruction, regRepo: RegisterRepository | null): string;
}

export declare class PcodePrinterJs extends PcodePrinter {
  printInstructionImpl: (pcode: PcodeInstruction) => void;

  printVarnodeImpl: (varnode: PcodeVarnode, printSizeAndOffset: boolean) => void;

  static New(regRepo: RegisterRepository | null): PcodePrinterJs;
}

module.exports = {
  ...module.exports,
  PcodeVarnode: m.PcodeVarnode,
  PcodeConstantVarnode: m.PcodeConstantVarnode,
  PcodeRegisterVarnode: m.PcodeRegisterVarnode,
  PcodeInstruction: m.PcodeInstruction,
  PcodeBlock: m.PcodeBlock,
  PcodeFunctionGraph: m.PcodeFunctionGraph,
  PcodeGraph: m.PcodeGraph,
  PcodeStructBlock: m.PcodeStructBlock,
  PcodeStructBlockSequence: m.PcodeStructBlockSequence,
  PcodeStructBlockIf: m.PcodeStructBlockIf,
  PcodeStructBlockWhile: m.PcodeStructBlockWhile,
  PcodeStructTree: m.PcodeStructTree,
  PcodeStructTreePrinter: m.PcodeStructTreePrinter,
  PcodeStructTreePrinterJs: m.PcodeStructTreePrinterJs,
  PcodeParser: m.PcodeParser,
  PcodePrinter: m.PcodePrinter,
  PcodePrinterJs: m.PcodePrinterJs,
};
