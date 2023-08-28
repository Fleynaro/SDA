import { ObjectId, Offset, TokenGroupAction, TokenizedText } from 'api/common';
import {
  IRcodeOperationTokenGroupAction,
  IRcodeTokenGroupAction,
  IRcodeFunction as IRcodeFunctionDto,
  IRcodeObjectId,
} from 'api/ir-code';
import {
  IRcodePrinterJs,
  IRcodePrinterToken,
  IRcodeProgram,
  AbstractPrinterToken,
  PcodeStructTree,
  RegisterRepository,
  IRcodeFunction,
  PcodeStructTreePrinterJs,
  PcodePrinterJs,
} from 'sda-core';
import { toHash, toId } from 'utils/common';
import { TokenWriter } from './common';

export const toIRcodeProgram = (id: ObjectId): IRcodeProgram => {
  const program = IRcodeProgram.Get(toHash(id));
  if (!program) {
    throw new Error(`Program ${id.key} does not exist`);
  }
  return program;
};

export const toIRcodeObjectId = (programId: ObjectId, offset: Offset): IRcodeObjectId => {
  return {
    programId,
    offset,
  };
};

export const toIRcodeFunctionDto = (func: IRcodeFunction): IRcodeFunctionDto => {
  const programId = toId(func.program);
  return {
    id: toIRcodeObjectId(programId, func.entryOffset),
  };
};

export const addIRcodePrinterToWriter = (printer: IRcodePrinterJs, writer: TokenWriter): void => {
  printer.printTokenImpl = (text, tokenType) => {
    writer.newToken(IRcodePrinterToken[tokenType] || AbstractPrinterToken[tokenType], text);
  };
  printer.printOperationImpl = (op) => {
    writer.newGroup(
      {
        name: IRcodeTokenGroupAction.Operation,
      } as IRcodeOperationTokenGroupAction,
      () => printer.printOperation(op),
    );
  };
  printer.printValueImpl = (value, extended) => {
    writer.newGroup(
      {
        name: IRcodeTokenGroupAction.Value,
      } as TokenGroupAction,
      () => printer.printValue(value, extended),
    );
  };
};

export const ircodeStructTreeToTokenizedText = (
  tree: PcodeStructTree,
  func: IRcodeFunction,
  regRepo: RegisterRepository | null,
): TokenizedText => {
  const writer = new TokenWriter();
  const printer = PcodeStructTreePrinterJs.New();
  const pcodePrinter = PcodePrinterJs.New(regRepo);
  const ircodePrinter = IRcodePrinterJs.New(pcodePrinter);
  ircodePrinter.combineWithStructPrinter(printer, func);
  addIRcodePrinterToWriter(ircodePrinter, writer);
  printer.printStructTree(tree);
  return writer.result;
};
