import { PcodeInstruction, PcodePrinter, PcodePrinterToken, RegisterRepository } from 'sda-core';
import { PcodeNode } from 'api/p-code';

export const toPcodeNode = (
  regRepo: RegisterRepository,
  instructions: PcodeInstruction[],
): PcodeNode => {
  const resultNode: PcodeNode = {
    type: 'group',
    childs: [],
  };

  const printerCtx = {
    curNode: resultNode,
    curIdx: 0,
  };

  const newNode = (node: PcodeNode, body?: () => void): void => {
    printerCtx.curNode.childs.push(node);
    const prevToken = printerCtx.curNode;
    if (node.type === 'group') {
      printerCtx.curNode = node;
      body?.();
      printerCtx.curNode = prevToken;
    }
  };

  const printer = PcodePrinter.New(regRepo);
  printer.printTokenImpl = (text, tokenType) => {
    newNode({
      type: 'token',
      text,
      tokenType: PcodePrinterToken[tokenType],
      idx: printerCtx.curIdx++,
    });
  };
  printer.printInstructionImpl = (instr) => {
    newNode(
      {
        type: 'group',
        childs: [],
        action: {
          name: 'instruction',
          offset: instr.offset,
        },
      },
      () => printer.printInstruction(instr),
    );
  };
  printer.printVarnodeImpl = (varnode, printSizeAndOffset) => {
    newNode(
      {
        type: 'group',
        childs: [],
        action: {
          name: 'varnode',
        },
      },
      () => printer.printVarnode(varnode, printSizeAndOffset),
    );
  };
  for (const instr of instructions) {
    printer.printInstructionImpl(instr);
  }

  return resultNode;
};
