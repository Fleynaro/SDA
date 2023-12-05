import { instance_of } from 'sda-bindings';
import { ObjectId, Offset, TokenizedText } from 'api/common';
import {
  IRcodeOperationTokenGroupAction,
  IRcodeTokenGroupAction,
  IRcodeFunction as IRcodeFunctionDto,
  IRcodeObjectId,
  IRcodeValueTokenGroupAction,
  IRcodeValueDto,
  IRcodeFunctionId,
  IRcodeVariableId,
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
  IRcodeVariable,
  IRcodeConstant,
  IRcodeRegister,
} from 'sda-core';
import { toHash, toId } from 'utils/common';
import { TokenWriter } from './common';
import { addPcodePrinterToWriter, addPcodeStructTreePrinterToWriter } from './p-code';
import assert from 'assert';

export const toIRcodeProgram = (id: ObjectId): IRcodeProgram => {
  const program = IRcodeProgram.Get(toHash(id));
  if (!program) {
    throw new Error(`Program ${id.key} does not exist`);
  }
  return program;
};

export const toIRcodeFunction = (functionId: IRcodeFunctionId): IRcodeFunction => {
  const program = toIRcodeProgram(functionId.programId);
  const func = program.getFunctionAt(functionId.offset);
  assert(func, `Function ${functionId.offset} does not exist`);
  return func;
};

export const toIRcodeVariable = (variableId: IRcodeVariableId): IRcodeVariable => {
  const func = toIRcodeFunction(variableId.functionId);
  const variable = func.findVariableById(variableId.variableId);
  assert(variable, `Variable ${variableId.variableId} does not exist`);
  return variable;
};

export const toIRcodeProgramId = toId;

export const toIRcodeObjectId = (programId: ObjectId, offset: Offset): IRcodeObjectId => {
  return {
    programId,
    offset,
  };
};

export const toIRcodeFunctionId = (func: IRcodeFunction): IRcodeFunctionId => {
  const programId = toIRcodeProgramId(func.program);
  return toIRcodeObjectId(programId, func.entryOffset);
};

export const toIRcodeVariableId = (variable: IRcodeVariable): IRcodeVariableId => {
  const func = variable.sourceOperation.block.function;
  return {
    functionId: toIRcodeFunctionId(func),
    variableId: variable.id,
  };
};

export const toIRcodeFunctionDto = (func: IRcodeFunction): IRcodeFunctionDto => {
  return {
    id: toIRcodeFunctionId(func),
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
        offset: op.pcodeInstruction?.offset,
        locatableByOffset: true,
      } as IRcodeOperationTokenGroupAction,
      () => printer.printOperation(op),
    );
  };
  printer.printValueImpl = (value, extended) => {
    let dto: IRcodeValueDto | undefined;
    if (instance_of(value, IRcodeVariable)) {
      dto = {
        type: 'variable',
        id: toIRcodeVariableId(value as IRcodeVariable),
        name: (value as IRcodeVariable).name,
      };
    } else if (instance_of(value, IRcodeConstant)) {
      dto = {
        type: 'constant',
      };
    } else if (instance_of(value, IRcodeRegister)) {
      dto = {
        type: 'register',
      };
    }
    writer.newGroup(
      {
        name: IRcodeTokenGroupAction.Value,
        value: dto,
      } as IRcodeValueTokenGroupAction,
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
  addPcodeStructTreePrinterToWriter(printer, writer);
  addPcodePrinterToWriter(pcodePrinter, writer);
  addIRcodePrinterToWriter(ircodePrinter, writer);
  printer.printStructTree(tree);
  return writer.result;
};
