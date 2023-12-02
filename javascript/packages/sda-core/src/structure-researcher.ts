import m from './module';
import { EventPipe } from './event';
import { IRcodeProgram } from './ir-code';
import { DataFlowNode, DataFlowRepository } from './data-flow-researcher';
import { ConstConditionRepository } from './const-condition-researcher';

export declare class ConstantSet {
  readonly values: { [offset: number]: number };
}

export declare class Structure {
  readonly name: string;
  readonly sourceNode: DataFlowNode;
  readonly linkedNodes: DataFlowNode[];
  readonly parents: Set<Structure>;
  readonly childs: Set<Structure>;
  readonly inputs: Set<Structure>;
  readonly outputs: Set<Structure>;
  readonly fields: { [offset: number]: Structure };
  readonly conditions: ConstantSet;
  readonly constants: ConstantSet;
}

export declare class StructureRepository {
  readonly allStructures: Structure[];
  readonly rootStructures: Structure[];

  getStructure(node: DataFlowNode): Structure;

  static New(eventPipe: EventPipe): StructureRepository;
}

export declare class StructureResearcher {
  readonly eventPipe: EventPipe;

  static New(
    program: IRcodeProgram,
    structureRepo: StructureRepository,
    dataFlowRepo: DataFlowRepository,
    constCondRepo: ConstConditionRepository,
  ): StructureResearcher;
}

export declare function PrintStructure(structure: Structure): string;

module.exports = {
  ...module.exports,
  ConstantSet: m.ConstantSet,
  Structure: m.Structure,
  StructureRepository: m.StructureRepository,
  StructureResearcher: m.StructureResearcher,
  PrintStructure: m.PrintStructure,
};
