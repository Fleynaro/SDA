import m from './module';
import { RegisterRepository } from './platform';
import { Offset, AbstractPrinter, AbstractPrinterToken } from './utils';

export declare abstract class PcodeVarnode {
  readonly size: number;
  readonly isRegister: boolean;
}

export type InstructionOffset = number;

export const toInstructionOffset = (byteOffset: Offset, index = 0): InstructionOffset =>
  (byteOffset << 8) | index;

export declare class PcodeInstruction {
  readonly id: string;
  readonly input0: PcodeVarnode;
  readonly input1: PcodeVarnode;
  readonly output: PcodeVarnode;
  readonly offset: InstructionOffset;
}

export declare class PcodeBlock {
  readonly level: number;
  readonly minOffset: InstructionOffset;
  readonly maxOffset: InstructionOffset;
  readonly nearNextBlock: PcodeBlock;
  readonly farNextBlock: PcodeBlock;
  readonly referencedBlocks: PcodeBlock[];
  readonly instructions: { [offset: InstructionOffset]: PcodeInstruction };

  contains(offset: InstructionOffset, halfInterval: boolean): boolean;
}

export declare class PcodeFunctionGraph {
  readonly entryBlock: PcodeBlock;
  readonly referencedGraphsTo: PcodeFunctionGraph[];
  readonly referencedGraphsFrom: PcodeFunctionGraph[];

  addReferencedGraphFrom(fromOffset: InstructionOffset, referencedGraph: PcodeFunctionGraph): void;

  removeReferencedGraphFrom(fromOffset: InstructionOffset): void;
}

export declare abstract class PcodeGraphCallbacks {
  onFunctionGraphCreated(graph: PcodeFunctionGraph): void;

  onFunctionGraphRemoved(graph: PcodeFunctionGraph): void;
}

export declare class PcodeGraphCallbacksImpl extends PcodeGraphCallbacks {
  prevCallbacks: PcodeGraphCallbacks;

  onFunctionGraphCreated: (graph: PcodeFunctionGraph) => void;

  onFunctionGraphRemoved: (graph: PcodeFunctionGraph) => void;

  static New(): PcodeGraphCallbacksImpl;
}

export declare class PcodeGraph {
  callbacks: PcodeGraphCallbacks;

  addInstruction(instruction: PcodeInstruction): void;

  removeInstruction(instruction: PcodeInstruction): void;

  getInstructionAt(offset: InstructionOffset): PcodeInstruction;

  createBlock(offset: InstructionOffset): PcodeBlock;

  removeBlock(block: PcodeBlock): void;

  getBlockAt(offset: InstructionOffset, halfInterval: boolean): PcodeBlock;

  createFunctionGraph(entryBlock: PcodeBlock): PcodeFunctionGraph;

  removeFunctionGraph(graph: PcodeFunctionGraph): void;
}

export declare class PcodeParser {
  static Parse(text: string, regRepo: RegisterRepository): PcodeInstruction[];
}

enum PcodePrinterToken_ {
  Mnemonic = AbstractPrinterToken.Keyword,
  Register = AbstractPrinterToken.Identifier,
  VirtRegister = AbstractPrinterToken.Identifier,
}

export type PcodePrinterToken = AbstractPrinterToken | PcodePrinterToken_;

export declare class PcodePrinter extends AbstractPrinter {
  printInstruction(pcode: PcodeInstruction): void;

  printInstructionImpl: (pcode: PcodeInstruction) => void;

  printVarnode(varnode: PcodeVarnode, printSizeAndOffset: boolean): void;

  printVarnodeImpl: (varnode: PcodeVarnode, printSizeAndOffset: boolean) => void;

  static New(regRepo: RegisterRepository): PcodePrinter;

  static Print(instruction: PcodeInstruction, regRepo: RegisterRepository): string;
}

module.exports = {
  ...module.exports,
  PcodeVarnode: m.PcodeVarnode,
  PcodeInstruction: m.PcodeInstruction,
  PcodeBlock: m.PcodeBlock,
  PcodeFunctionGraph: m.PcodeFunctionGraph,
  PcodeGraphCallbacks: m.PcodeGraphCallbacks,
  PcodeGraphCallbacksImpl: m.PcodeGraphCallbacksImpl,
  PcodeGraph: m.PcodeGraph,
  PcodeParser: m.PcodeParser,
  PcodePrinter: m.PcodePrinter,
};
