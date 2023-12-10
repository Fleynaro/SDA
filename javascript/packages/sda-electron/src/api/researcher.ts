import { ObjectId, Offset, window_ } from './common';
import { IRcodeFunctionId, IRcodeVariableId } from './ir-code';

export type ConstantSet = { [offset: Offset]: number[] };

export type StructureId = {
  programId: ObjectId;
  name: string;
};

export type Structure = {
  id: StructureId;
  name: string;
  parents: StructureId[];
  children: StructureId[];
  inputs: StructureId[];
  outputs: StructureId[];
  fields: { [offset: Offset]: StructureId };
  conditions: ConstantSet;
  constants: ConstantSet;
  classInfo?: {
    labels: number[];
    labelOffset: Offset;
    labelSet: ConstantSet;
    structuresInGroup: StructureId[];
  };
};

export type StructureLink = {
  structure: Structure;
  offset: Offset;
  own: boolean;
};

export type SemanticsId = {
  programId: ObjectId;
  hash: string;
};

export type Semantics = {
  name: string;
  predecessors: SemanticsId[];
  successors: SemanticsId[];
  holder?: {
    semantics: Semantics;
    object: SemanticsObject;
  };
};

export type SemanticsObject = {
  semantics: SemanticsId[];
  variables: IRcodeVariableId[];
};

export interface ResearcherController {
  findStructureByVariableId(variableId: IRcodeVariableId): Promise<StructureLink | undefined>;

  getStructureById(structureId: StructureId): Promise<Structure>;

  printStructure(structureId: StructureId): Promise<string>;

  findSemanticsObjectByVariableId(
    variableId: IRcodeVariableId,
  ): Promise<SemanticsObject | undefined>;

  getSemanticsById(semanticsId: SemanticsId): Promise<Semantics>;

  printDataFlowForFunction(functionId: IRcodeFunctionId): Promise<string>;
}

export const getResearcherApi = () => {
  return window_.researcherApi as ResearcherController;
};
