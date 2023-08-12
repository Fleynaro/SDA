import {
  PcodeInstruction,
  PcodePrinterToken,
  RegisterRepository,
  AbstractPrinterToken,
  PcodePrinterJs,
  PcodeStructTreePrinterJs,
  PcodeStructTree,
} from 'sda-core';
import { PcodeInstructionTokenGroupAction, PcodeTokenGroupAction } from 'api/p-code';
import { TokenGroupAction, TokenizedText } from 'api/common';
import { TokenWriter } from './common';

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
        name: 'struct_block',
      } as TokenGroupAction,
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
  printer.setPcodePrinter(pcodePrinter);
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
