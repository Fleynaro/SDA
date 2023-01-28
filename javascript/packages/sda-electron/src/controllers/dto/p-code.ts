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
    groups: [
      {
        idx: 0,
        action: {
          name: 'root',
        },
      },
    ],
  };

  const printerCtx = {
    curGroupIdx: 1,
    curGroup: result.groups[0],
  };
  const newToken = (type: string, text: string): void => {
    result.tokens.push({
      groupIdx: printerCtx.curGroup.idx,
      type,
      text,
    });
  };
  const newGroup = (action: PcodeGroup['action'], body?: () => void): void => {
    const prevGroup = printerCtx.curGroup;
    const newGroup: PcodeGroup = {
      idx: printerCtx.curGroupIdx++,
      action,
    };
    result.groups.push(newGroup);
    printerCtx.curGroup = newGroup;
    body?.();
    printerCtx.curGroup = prevGroup;
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
