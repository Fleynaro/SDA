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

export interface ResearcherController {
  findStructureByVariableId(variableId: IRcodeVariableId): Promise<StructureLink | undefined>;

  getStructureById(structureId: StructureId): Promise<Structure>;

  printStructure(structureId: StructureId): Promise<string>;

  printDataFlowForFunction(functionId: IRcodeFunctionId): Promise<string>;
}

export const getResearcherApi = () => {
  return window_.researcherApi as ResearcherController;
};
