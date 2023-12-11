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
  PcodeRegisterVarnode,
  PcodeConstantVarnode,
} from 'sda-core';
import {
  PcodeInstructionTokenGroupAction,
  PcodeObjectId,
  PcodeTokenGroupAction,
  PcodeBlock as PcodeBlockDto,
  PcodeFunctionGraph as PcodeFunctionGraphDto,
  PcodeStructBlockTokenGroupAction,
  PcodeVarnodeDto,
  PcodeVarnodeTokenGroupAction,
} from 'api/p-code';
import { ObjectId, Offset, TokenizedText } from 'api/common';
import { TokenWriter } from './common';
import { toHash, toId } from 'utils/common';
import { instance_of } from 'sda-bindings';

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
        locatableByOffset: true,
      } as PcodeInstructionTokenGroupAction,
      () => printer.printInstruction(instr),
    );
  };
  printer.printVarnodeImpl = (varnode, printSizeAndOffset) => {
    let dto: PcodeVarnodeDto | undefined;
    if (instance_of(varnode, PcodeRegisterVarnode)) {
      dto = {
        type: 'register',
      };
    } else if (instance_of(varnode, PcodeConstantVarnode)) {
      dto = {
        type: 'constant',
        value: (varnode as PcodeConstantVarnode).value,
      };
    }
    writer.newGroup(
      {
        name: PcodeTokenGroupAction.Varnode,
        varnode: dto,
      } as PcodeVarnodeTokenGroupAction,
      () => printer.printVarnode(varnode, printSizeAndOffset),
    );
  };
};

export const pcodeStructTreeToTokenizedText = (
  tree: PcodeStructTree,
  regRepo: RegisterRepository | null,
) => {
  const writer = new TokenWriter();
  const structPrinter = PcodeStructTreePrinterJs.New();
  const pcodePrinter = PcodePrinterJs.New(regRepo);
  pcodePrinter.combineWithStructPrinter(structPrinter);
  addPcodeStructTreePrinterToWriter(structPrinter, writer);
  addPcodePrinterToWriter(pcodePrinter, writer);
  return {
    structPrinter,
    pcodePrinter,
    print: () => {
      structPrinter.printStructTree(tree);
      return writer.result;
    },
  };
};

export const instructionsToTokenizedText = (
  instructions: PcodeInstruction[],
  regRepo: RegisterRepository | null,
) => {
  const writer = new TokenWriter();
  const pcodePrinter = PcodePrinterJs.New(regRepo);
  addPcodePrinterToWriter(pcodePrinter, writer);
  return {
    pcodePrinter,
    print: () => {
      for (const instr of instructions) {
        pcodePrinter.printInstructionImpl(instr);
        pcodePrinter.newLine();
      }
      return writer.result;
    },
  };
};
