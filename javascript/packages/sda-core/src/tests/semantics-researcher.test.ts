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
import { DataFlowCollector, DataFlowRepository } from '../data-flow-researcher';
import { SignatureRepository, SignatureResearcher } from '../signature-researcher';
import { ConstConditionRepository } from '../const-condition-researcher';
import { StructureRepository, StructureResearcher } from '../structure-researcher';
import { ClassRepository, ClassResearcher } from '../class-researcher';
import {
  BaseSemanticsPropagator,
  SemanticsRepository,
  SemanticsResearcher,
} from '../semantics-researcher';
import { stripSpaces } from './helpers';

describe('Semantics researcher', () => {
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
  let classRepo: ClassRepository;
  let classResearcher: ClassResearcher;
  let semanticsRepo: SemanticsRepository;
  let semanticsResearcher: SemanticsResearcher;
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
    classRepo = ClassRepository.New(pipe);
    classResearcher = ClassResearcher.New(program, classRepo, structureRepo);
    pipe.connect(classResearcher.eventPipe);
    semanticsRepo = SemanticsRepository.New(pipe);
    semanticsResearcher = SemanticsResearcher.New(program, semanticsRepo, classRepo, dataFlowRepo);
    const semanticsPropagator = BaseSemanticsPropagator.New(program, semanticsRepo, dataFlowRepo);
    semanticsResearcher.addPropagator(semanticsPropagator);
    pipe.connect(semanticsResearcher.eventPipe);

    // analyze p-code
    {
      const source = `
        $1:8 = INT_ADD $0:8, 0x10:8
        $2:4 = FLOAT_MULT 1:4, 2:4
        STORE $1:8, $2:4
      `;
      const instructions = PcodeParser.Parse(source, null);
      graph.exploreInstructions(0, instructions);
      entryFunction = program.getFunctionAt(0)!;
    }
  });

  it('should print ir-code', () => {
    const expected = `
      Block B0(level: 1):
        var1:8 = LOAD $U0
        var2[$U1]:8 = INT_ADD var1, 0x10:8
        var3[$U2]:4 = FLOAT_MULT 0x1:4, 0x2:4
        var4[var2]:4 = COPY var3
    `;
    const actual = PrintIRcode(entryFunction, 2, false);
    expect(stripSpaces(actual)).toBe(stripSpaces(expected));
  });

  it('should access fields', () => {
    const var3 = entryFunction.findVariableById(3)!;
    const semObj = semanticsRepo.getObject(var3)!;
    expect(semObj.variables).toHaveLength(2);
    expect([semObj.variables[0].name, semObj.variables[1].name].sort()).toEqual(['var3', 'var4']);
    expect(semObj.semantics.length).toBeGreaterThan(0);
    expect(semObj.semantics.some((sem) => sem.semantics.name === 'float')).toBe(true);
  });
});
