import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import {
  GotoType,
  PcodeGraph,
  PcodeInstructionId,
  PcodeParser,
  PcodePrinterJs,
  PcodeStructBlock,
  PcodeStructBlockIf,
  PcodeStructBlockSequence,
  PcodeStructBlockWhile,
  PcodeStructTree,
  PcodeStructTreePrinterJs,
  toInstructionOffset,
} from '../p-code';
import { instance_of } from 'sda-bindings';
import { AbstractPrinterToken } from '../utils';

describe('P-code', () => {
  let context: Context;

  beforeEach(() => {
    const pipe = EventPipe.New('test');
    const platform = PlatformMock.New();
    context = Context.New(pipe, platform);
  });

  it('parser/printer', () => {
    const source = `
      $1:8 = COPY 0:8
      CBRANCH <labelElse>, 0:1 // if-else condition
      $1:8 = COPY 1:8 // then block
      BRANCH <labelEnd>
      <labelElse>:
      NOP // else block
      <labelEnd>:
      $2:8 = INT_2COMP $1:8
    `;
    const expected = `
      $U1(8) = COPY 0x0(8)
      CBRANCH 0x400(8), 0x0(1)
      $U1(8) = COPY 0x1(8)
      BRANCH 0x500(8)
      NOP
      $U2(8) = INT_2COMP $U1(8)
    `;
    const instructions = PcodeParser.Parse(source, null);
    const printer = PcodePrinterJs.New(null);
    printer.printVarnodeImpl = (varnode) => {
      // replace :size with (size)
      printer.printVarnode(varnode, false);
      printer.printToken(`(${varnode.size})`, AbstractPrinterToken.Number);
    };
    for (const instr of instructions) {
      printer.printInstruction(instr);
      printer.newLine();
    }
    expect(printer.output.replace(/\s/g, '')).toBe(expected.replace(/\s/g, ''));
  });

  it('explore', () => {
    const source = `
      $1:8 = COPY 0:8
      CBRANCH <labelElse>, 0:1 // if-else condition
      $1:8 = COPY 1:8 // then block
      BRANCH <labelEnd>
      <labelElse>:
      NOP // else block
      <labelEnd>:
      $2:8 = INT_2COMP $1:8
    `;
    const instructions = PcodeParser.Parse(source, null);
    const pcodeGraph = PcodeGraph.New(context.eventPipe, context.platform);
    pcodeGraph.exploreInstructions(0, instructions);
    expect(pcodeGraph.getFunctionGraphAt(toInstructionOffset(0x0)).name).toBe('B0');
    expect(pcodeGraph.getBlockAt(toInstructionOffset(0x5), false).name).toBe('B5');
    expect(pcodeGraph.getInstructionAt(toInstructionOffset(0x5)).id).toBe(
      PcodeInstructionId.INT_2COMP,
    );
  });

  it('structurer', () => {
    const source = `
      <loop>:
      NOP
      CBRANCH <loop>, 0x0:1
      RETURN
    `;
    const expected = `
      // this is a while
      while (true) {
        // Block B0
        NOP
        if (!0x0:1) {
            break;
        }
      }
      // Block B2
      RETURN
    `;
    const instructions = PcodeParser.Parse(source, null);
    const pcodeGraph = PcodeGraph.New(context.eventPipe, context.platform);
    pcodeGraph.exploreInstructions(0, instructions);
    const funcGraph = pcodeGraph.getFunctionGraphAt(toInstructionOffset(0x0));

    const structTree = PcodeStructTree.New();
    structTree.init(funcGraph);
    expect(instance_of(structTree.entryBlock, PcodeStructBlockSequence)).toBe(true);
    const seq = structTree.entryBlock as PcodeStructBlockSequence;
    expect(seq.blocks.length).toBe(2);
    expect(instance_of(seq.blocks[0], PcodeStructBlockWhile)).toBe(true);
    expect(instance_of(seq.blocks[1], PcodeStructBlock)).toBe(true);
    const whileBlock = seq.blocks[0] as PcodeStructBlockWhile;
    expect(instance_of(whileBlock.bodyBlock, PcodeStructBlockIf)).toBe(true);
    const ifBlock = whileBlock.bodyBlock as PcodeStructBlockIf;
    expect(instance_of(ifBlock.conditionBlock, PcodeStructBlock)).toBe(true);
    expect(ifBlock.conditionBlock.name).toBe('B0');
    expect(ifBlock.thenBlock.gotoType).toBe(GotoType.Break);

    // check printer
    const printer = PcodeStructTreePrinterJs.New();
    const pcodePrinter = PcodePrinterJs.New(null);
    printer.setPcodePrinter(pcodePrinter);
    printer.printStructBlockImpl = (block: PcodeStructBlock) => {
      if (instance_of(block, PcodeStructBlockWhile)) {
        printer.printToken('// this is a while', AbstractPrinterToken.Comment);
        printer.newLine();
      }
      printer.printStructBlock(block);
    };
    printer.printStructTree(structTree);
    expect(printer.output.replace(/\s/g, '')).toBe(expected.replace(/\s/g, ''));
  });
});
