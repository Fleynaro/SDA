import m from './module';
import { EventPipe } from './event';
import { IRcodeProgram } from './ir-code';
import { ConstantSet, Structure, StructureRepository } from './structure-researcher';
import { InstructionOffset } from './p-code';
import { Offset } from './utils';

export declare class FieldStructureGroup {
  readonly structures: Set<Structure>;
}

export declare class StructureInfo {
  readonly labelOffset: Offset;
  readonly labels: Set<number>;
  readonly labelSet: ConstantSet;
  readonly parents: Set<Structure>;
  readonly childs: Set<Structure>;
  readonly inputs: Set<Structure>;
  readonly outputs: Set<Structure>;
  readonly group?: FieldStructureGroup;
}

export interface ClassLabelInfo {
  structureInstrOffset: InstructionOffset;
  sourceInstrOffset: InstructionOffset;
  labelOffset: Offset;
}

export declare class ClassRepository {
  readonly eventPipe: EventPipe;

  getStructureInfo(structure: Structure): StructureInfo | undefined;

  addUserDefinedLabelOffset(structure: Structure, info: ClassLabelInfo): void;

  static New(eventPipe: EventPipe): ClassRepository;
}

export declare class ClassResearcher {
  readonly eventPipe: EventPipe;

  static New(
    program: IRcodeProgram,
    classRepo: ClassRepository,
    structureRepo: StructureRepository,
  ): ClassResearcher;
}

module.exports = {
  ...module.exports,
  FieldStructureGroup: m.FieldStructureGroup,
  StructureInfo: m.StructureInfo,
  ClassRepository: m.ClassRepository,
  ClassResearcher: m.ClassResearcher,
};
