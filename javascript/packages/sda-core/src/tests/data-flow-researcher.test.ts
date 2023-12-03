import { CustomCallingConvention, PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { PcodeGraph, PcodeParser } from '../p-code';
import {
  IRcodeContextSync,
  IRcodeFunction,
  IRcodePcodeSync,
  IRcodeProgram,
  PrintIRcode,
} from '../ir-code';
import { StandartSymbolTable } from '../symbol-table';
import {
  DataFlowCollector,
  DataFlowNodeType,
  DataFlowRepository,
  PrintDataFlowForFunction,
} from '../data-flow-researcher';
import { stripSpaces } from './helpers';

describe('Data flow researcher', () => {
  let context: Context;
  let graph: PcodeGraph;
  let program: IRcodeProgram;
  let ctxSync: IRcodeContextSync;
  let pcodeSync: IRcodePcodeSync;
  let dataFlowRepo: DataFlowRepository;
  let dataFlowResearcher: DataFlowCollector;
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
    dataFlowRepo = DataFlowRepository.New(pipe);
    dataFlowResearcher = DataFlowCollector.New(program, dataFlowRepo);
    pipe.connect(dataFlowResearcher.eventPipe);

    // analyze p-code
    {
      const source = `
        $1:8 = INT_ADD $0:8, 0x10:8
        STORE $1:8, $100:4
        $2:4 = LOAD $1:8, 4:8
      `;
      const instructions = PcodeParser.Parse(source, null);
      graph.exploreInstructions(0, instructions);
      entryFunction = program.getFunctionAt(0);
    }
  });

  it('should print ir-code', () => {
    const expected = `
      Block B0(level: 1):
        var1:8 = LOAD $U0
        var2[$U1]:8 = INT_ADD var1, 0x10:8
        var3:4 = LOAD $U100
        var4[var2]:4 = COPY var3
        var5[$U2]:4 = COPY var4
    `;
    const actual = PrintIRcode(entryFunction, 2, false);
    expect(stripSpaces(actual)).toBe(stripSpaces(expected));
  });

  it('should print data flow', () => {
    const expected = `
      var1 <- Unknown
      var2 <- Copy var1 + 0x10
      var3 <- Unknown
      var4 <- Write var2
      var4 <- Write var3
      var5 <- Copy var4
    `;
    const actual = PrintDataFlowForFunction(dataFlowRepo, entryFunction);
    expect(stripSpaces(actual)).toBe(stripSpaces(expected));
  });

  it('should access nodes', () => {
    expect(dataFlowRepo.globalStartNode.type).toBe(DataFlowNodeType.Start);
    const var2 = entryFunction.findVariableById(2);
    const nodeOfVar2 = dataFlowRepo.getNode(var2);
    expect(nodeOfVar2.type).toBe(DataFlowNodeType.Copy);
    expect(nodeOfVar2.name).toBe('B0:var2');
    expect(nodeOfVar2.predecessors.length).toBe(1);
    expect(nodeOfVar2.successors.length).toBe(1);
    const var4 = entryFunction.findVariableById(4);
    const nodeOfVar4 = dataFlowRepo.getNode(var4);
    expect(nodeOfVar4.type).toBe(DataFlowNodeType.Write);
    expect(nodeOfVar4.name).toBe('B0:var4');
    expect(nodeOfVar4.predecessors.length).toBe(2);
  });
});
