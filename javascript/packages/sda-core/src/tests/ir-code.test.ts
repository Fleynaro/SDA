import { CustomCallingConvention, PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { PcodeGraph, PcodeParser, PcodePrinterJs } from '../p-code';
import {
  IRcodeContextSync,
  IRcodeFunction,
  IRcodeOperationId,
  IRcodePcodeSync,
  IRcodePrinterJs,
  IRcodeProgram,
} from '../ir-code';
import { StandartSymbolTable } from '../symbol-table';
import { AbstractPrinterToken } from '../utils';

describe('IR-code', () => {
  let context: Context;
  let graph: PcodeGraph;
  let program: IRcodeProgram;
  let ctxSync: IRcodeContextSync;
  let pcodeSync: IRcodePcodeSync;
  let entryFunction: IRcodeFunction;

  beforeAll(() => {
    // create objects
    const pipe = EventPipe.New('test');
    const platform = PlatformMock.New();
    context = Context.New(pipe, platform);
    graph = PcodeGraph.New(pipe, platform);
    const symbolTable = StandartSymbolTable.New(context, 'test');
    program = IRcodeProgram.New(graph, symbolTable);
    const callingConv = CustomCallingConvention.New({});
    ctxSync = IRcodeContextSync.New(program, symbolTable, callingConv);
    pcodeSync = IRcodePcodeSync.New(program);
    pipe.connect(ctxSync.eventPipe);
    pipe.connect(pcodeSync.eventPipe);

    // analyze p-code
    {
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
      graph.exploreInstructions(0, instructions);
      entryFunction = program.getFunctionAt(0);
    }
  });

  it('should access fields', () => {
    const operations = entryFunction.entryBlock.operations;
    expect(operations).toHaveLength(1);
    expect(operations[0].id).toBe(IRcodeOperationId.COPY);
  });

  it('should print the first IR-code operation in the entry block', () => {
    const operations = entryFunction.entryBlock.operations;
    const pcodePrinter = PcodePrinterJs.New(null);
    const ircodePrinter = IRcodePrinterJs.New(pcodePrinter);
    ircodePrinter.printValueImpl = (value, extended) => {
      ircodePrinter.printToken('<', AbstractPrinterToken.Keyword);
      ircodePrinter.printValue(value, extended);
      ircodePrinter.printToken('>', AbstractPrinterToken.Keyword);
    };
    ircodePrinter.printOperation(operations[0]);
    expect(ircodePrinter.output).toBe('<var1[<$U1>]:8> = COPY <0x0:8>');
  });
});
