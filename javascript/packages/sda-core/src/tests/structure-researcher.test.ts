import { CustomCallingConvention, PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { PcodeGraph, PcodeParser } from '../p-code';
import { IRcodeContextSync, IRcodeFunction, IRcodePcodeSync, IRcodeProgram } from '../ir-code';
import { StandartSymbolTable } from '../symbol-table';
import { DataFlowCollector, DataFlowRepository } from '../data-flow-researcher';
import { stripSpaces } from './helpers';
import { SignatureRepository, SignatureResearcher } from '../signature-researcher';
import { ConstConditionRepository } from '../const-condition-researcher';
import { PrintStructure, StructureRepository, StructureResearcher } from '../structure-researcher';

describe('Structure researcher', () => {
  let context: Context;
  let graph: PcodeGraph;
  let program: IRcodeProgram;
  let ctxSync: IRcodeContextSync;
  let pcodeSync: IRcodePcodeSync;
  let dataFlowRepo: DataFlowRepository;
  let dataFlowResearcher: DataFlowCollector;
  let signatureRepo: SignatureRepository;
  let signatureResearcher: SignatureResearcher;
  let constConditionRepo: ConstConditionRepository;
  let structureRepo: StructureRepository;
  let structureResearcher: StructureResearcher;
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
    signatureRepo = SignatureRepository.New();
    signatureResearcher = SignatureResearcher.New(program, callingConv, signatureRepo);
    pipe.connect(signatureResearcher.eventPipe);
    constConditionRepo = ConstConditionRepository.New(program);
    pipe.connect(constConditionRepo.eventPipe);
    structureRepo = StructureRepository.New(pipe);
    structureResearcher = StructureResearcher.New(
      program,
      structureRepo,
      dataFlowRepo,
      constConditionRepo,
    );
    pipe.connect(structureResearcher.eventPipe);

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

  it('should print structure', () => {
    const expected = `
      struct B0:var1 {
        0x10: B0:var3
      }
    `;
    const var1 = entryFunction.findVariableById(1);
    const node = dataFlowRepo.getNode(var1);
    const structure = structureRepo.getStructure(node);
    const actual = PrintStructure(structure);
    expect(stripSpaces(actual)).toBe(stripSpaces(expected));
  });

  it('should access fields', () => {
    const var1 = entryFunction.findVariableById(1);
    const node = dataFlowRepo.getNode(var1);
    const structure = structureRepo.getStructure(node);
    expect(structure.name).toBe('B0:var1');
    const fieldStructure = structure.fields[0x10];
    expect(fieldStructure).toBeDefined();
    expect(fieldStructure.name).toBe('B0:var1_0x10');
    expect(fieldStructure.childs.size).toBe(1);
    const childs = [...fieldStructure.childs];
    expect(childs[0].name).toBe('B0:var3');
  });
});
