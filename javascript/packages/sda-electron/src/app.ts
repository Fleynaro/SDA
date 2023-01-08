import { findPlatform } from './repo/platform';
import { app } from 'electron';
import path from 'path';
import { existsSync, mkdirSync } from 'fs';
import { initControllers } from './controllers';
import { Program, ProgramCallbacksImpl, CleanUpSharedObjectLookupTable } from 'sda';
import { initDefaultPlatforms } from 'repo/platform';
import { initDefaultImageAnalysers } from 'repo/image-analyser';
import { objectChangeEmitter, ObjectChangeType } from './eventEmitter';
import { toId } from 'utils/common';
import { PcodeParser, PcodePrinter } from 'sda-core';
import { PcodeToken } from 'api/p-code';

export let program: Program;

export const getUserDir = () => {
  return path.join(app.getPath('documents'), 'SDA');
};

export const getUserPath = (file: string) => {
  return path.join(getUserDir(), file);
};

export const initApp = () => {
  program = Program.New();
  {
    const callbacks = ProgramCallbacksImpl.New();
    callbacks.prevCallbacks = program.callbacks;
    callbacks.onProjectAdded = (project) =>
      objectChangeEmitter()(toId(project), ObjectChangeType.Create);
    callbacks.onProjectRemoved = (project) =>
      objectChangeEmitter()(toId(project), ObjectChangeType.Delete);
    program.callbacks = callbacks;
  }
  initDefaultPlatforms();
  initDefaultImageAnalysers();
  initControllers();

  // create user directory if not exists
  const userDir = getUserDir();
  if (!existsSync(userDir)) {
    mkdirSync(userDir);
  }

  setInterval(() => {
    // TODO: remove when app exit
    CleanUpSharedObjectLookupTable();
  }, 10000);

  test4();
};

function test4() {
  const pcodeStr =
    '\
      rcx:8 = COPY rcx:8 \
      rbx:8 = INT_MULT rdx:8, 4:8 \
      rbx:8 = INT_ADD rcx:8, rbx:8 \
      rbx:8 = INT_ADD rbx:8, 0x10:8 \
      STORE rbx:8, 1.0:8 \
  ';
  const platformX86 = findPlatform('x86-64');
  const instrs = PcodeParser.Parse(pcodeStr, platformX86.registerRepository);
  const instr = instrs[0];
  console.log('Pcode instruction = ' + instr.id);
  console.log('input0 = ' + instr.input0.size + ', ' + instr.input0.isRegister);

  const printer = PcodePrinter.New(platformX86.registerRepository);
  const pcodeTextObj: PcodeToken = {
    group: true,
    tokens: [],
  };
  const printerCtx = {
    pcodeTextObj,
  };
  printer.printTokenImpl = (text) => {
    console.log('token = ' + text);
  };
  printer.printInstructionImpl = (instr) => {
    console.log('instruction = ' + instr.id);
    printer.printInstruction(instr);
  };
  printer.printVarnodeImpl = (varnode, printSizeAndOffset) => {
    console.log('varnode = ' + varnode.size + ', ' + varnode.isRegister);
    printer.printVarnode(varnode, printSizeAndOffset);
  };
  printer.flush();
  printer.printInstructionImpl(instr);
  console.log(printer.output);
}
