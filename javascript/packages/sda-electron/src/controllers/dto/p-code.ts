import {
  PcodeInstruction,
  PcodePrinterToken,
  RegisterRepository,
  AbstractPrinterToken,
  PcodePrinterJs,
  PcodeStructTreePrinterJs,
  PcodeStructTree,
  PcodeGraph,
  PcodeBlock,
  PcodeFunctionGraph,
} from 'sda-core';
import {
  PcodeInstructionTokenGroupAction,
  PcodeObjectId,
  PcodeTokenGroupAction,
  PcodeBlock as PcodeBlockDto,
  PcodeFunctionGraph as PcodeFunctionGraphDto,
  PcodeStructBlockTokenGroupAction,
} from 'api/p-code';
import { ObjectId, Offset, TokenGroupAction, TokenizedText } from 'api/common';
import { TokenWriter } from './common';
import { toHash, toId } from 'utils/common';

export const toPcodeGraph = (id: ObjectId): PcodeGraph => {
  const graph = PcodeGraph.Get(toHash(id));
  if (!graph) {
    throw new Error(`Pcode graph ${id.key} does not exist`);
  }
  return graph;
};

export const toPcodeObjectId = (graphId: ObjectId, offset: Offset): PcodeObjectId => {
  return {
    graphId,
    offset,
  };
};

export const toFunctionGraphDto = (functionGraph: PcodeFunctionGraph): PcodeFunctionGraphDto => {
  const graphId = toId(functionGraph.graph);
  return {
    id: toPcodeObjectId(graphId, functionGraph.entryOffset),
  };
};

export const toPcodeBlockDto = (block: PcodeBlock): PcodeBlockDto => {
  const graphId = toId(block.graph);
  return {
    id: toPcodeObjectId(graphId, block.minOffset),
    functionGraph: toFunctionGraphDto(block.functionGraph),
  };
};

export const addPcodeStructTreePrinterToWriter = (
  printer: PcodeStructTreePrinterJs,
  writer: TokenWriter,
): void => {
  printer.printTokenImpl = (text, tokenType) => {
    writer.newToken(AbstractPrinterToken[tokenType], text);
  };
  printer.printStructBlockImpl = (block) => {
    writer.newGroup(
      {
        name: PcodeTokenGroupAction.StructBlock,
        id: block.name,
      } as PcodeStructBlockTokenGroupAction,
      () => printer.printStructBlock(block),
    );
  };
};

export const addPcodePrinterToWriter = (printer: PcodePrinterJs, writer: TokenWriter): void => {
  printer.printTokenImpl = (text, tokenType) => {
    writer.newToken(PcodePrinterToken[tokenType] || AbstractPrinterToken[tokenType], text);
  };
  printer.printInstructionImpl = (instr) => {
    writer.newGroup(
      {
        name: PcodeTokenGroupAction.Instruction,
        offset: instr.offset,
      } as PcodeInstructionTokenGroupAction,
      () => printer.printInstruction(instr),
    );
  };
  printer.printVarnodeImpl = (varnode, printSizeAndOffset) => {
    writer.newGroup(
      {
        name: PcodeTokenGroupAction.Varnode,
      } as TokenGroupAction,
      () => printer.printVarnode(varnode, printSizeAndOffset),
    );
  };
};

export const pcodeStructTreeToTokenizedText = (
  tree: PcodeStructTree,
  regRepo: RegisterRepository | null,
): TokenizedText => {
  const writer = new TokenWriter();
  const printer = PcodeStructTreePrinterJs.New();
  const pcodePrinter = PcodePrinterJs.New(regRepo);
  pcodePrinter.combineWithStructPrinter(printer);
  addPcodeStructTreePrinterToWriter(printer, writer);
  addPcodePrinterToWriter(pcodePrinter, writer);
  printer.printStructTree(tree);
  return writer.result;
};

export const instructionsToTokenizedText = (
  instructions: PcodeInstruction[],
  regRepo: RegisterRepository | null,
): TokenizedText => {
  const writer = new TokenWriter();
  const printer = PcodePrinterJs.New(regRepo);
  addPcodePrinterToWriter(printer, writer);
  for (const instr of instructions) {
    printer.printInstructionImpl(instr);
    printer.newLine();
  }
  return writer.result;
};
