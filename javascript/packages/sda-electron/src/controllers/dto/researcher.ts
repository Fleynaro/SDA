import { ConstantSet, IRcodeProgram, Structure, StructureInfo } from 'sda-core';
import {
  ConstantSet as ConstantSetDto,
  Structure as StructureDto,
  StructureId,
} from 'api/researcher';
import { getImageInfoByProgram } from 'repo/image';
import { toIRcodeProgram, toIRcodeProgramId } from './ir-code';
import assert from 'assert';

export const toConstantSet = (set: ConstantSet) => {
  return Object.entries(set.values).reduce((acc, [offset, values]) => {
    acc[offset] = [...values];
    return acc;
  }, {} as ConstantSetDto);
};

export const toStructure = (structureId: StructureId): Structure => {
  const program = toIRcodeProgram(structureId.programId);
  const { researchers } = getImageInfoByProgram(program);
  const structure = researchers.structureRepo.getStructureByName(structureId.name);
  assert(structure, `Structure ${structureId.name} does not exist`);
  return structure;
};

export const toStructureId = (program: IRcodeProgram, structure: Structure): StructureId => {
  return {
    programId: toIRcodeProgramId(program),
    name: structure.name,
  };
};

export const toStructureDto = (
  program: IRcodeProgram,
  structure: Structure,
  info?: StructureInfo,
): StructureDto => {
  const parents = info ? info.parents : structure.parents;
  const childs = info ? info.childs : structure.childs;
  const inputs = info ? info.inputs : structure.inputs;
  const outputs = info ? info.outputs : structure.outputs;
  return {
    id: toStructureId(program, structure),
    name: structure.name,
    parents: [...parents].map((s) => toStructureId(program, s)),
    children: [...childs].map((s) => toStructureId(program, s)),
    inputs: [...inputs].map((s) => toStructureId(program, s)),
    outputs: [...outputs].map((s) => toStructureId(program, s)),
    fields: Object.entries(structure.fields).reduce((acc, [offset, fieldStructure]) => {
      acc[offset] = toStructureId(program, fieldStructure);
      return acc;
    }, {}),
    conditions: toConstantSet(structure.conditions),
    constants: toConstantSet(structure.constants),
    classInfo: info && {
      labels: [...info.labels],
      labelOffset: info.labelOffset,
      labelSet: toConstantSet(info.labelSet),
      structuresInGroup: (info.group ? [...info.group.structures] : []).map((s) =>
        toStructureId(program, s),
      ),
    },
  };
};
