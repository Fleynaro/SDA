import {
  PcodeInstruction,
  PcodePrinter,
  PcodePrinterToken,
  RegisterRepository,
  AbstractPrinterToken,
} from 'sda-core';
import { PcodeGroup, PcodeText } from 'api/p-code';

export const toPcodeText = (
  regRepo: RegisterRepository,
  instructions: PcodeInstruction[],
): PcodeText => {
  const result: PcodeText = {
    tokens: [],
    groups: [],
  };

  const printerCtx = {
    curIdx: 0,
  };
  const newToken = (type: string, text: string): void => {
    result.tokens.push({
      idx: printerCtx.curIdx++,
      type,
      text,
    });
  };
  const newGroup = (action?: PcodeGroup['action'], body?: () => void): void => {
    const start = printerCtx.curIdx;
    body?.();
    const end = printerCtx.curIdx;
    result.groups.push({
      start,
      end,
      action,
    });
  };

  const printer = PcodePrinter.New(regRepo);
  printer.printTokenImpl = (text, tokenType) => {
    newToken(PcodePrinterToken[tokenType] || AbstractPrinterToken[tokenType], text);
  };
  printer.printInstructionImpl = (instr) => {
    newGroup(
      {
        name: 'instruction',
        offset: instr.offset,
      },
      () => printer.printInstruction(instr),
    );
  };
  printer.printVarnodeImpl = (varnode, printSizeAndOffset) => {
    newGroup(
      {
        name: 'varnode',
      },
      () => printer.printVarnode(varnode, printSizeAndOffset),
    );
  };
  for (const instr of instructions) {
    printer.printInstructionImpl(instr);
    printer.newLine();
  }

  return result;
};
