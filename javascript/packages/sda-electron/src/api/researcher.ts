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
  conditions: ConstantSet;
  constants: ConstantSet;
};

export type StructureInfo = {
  structure: Structure;
  offset: number;
  own: boolean;
};

export interface ResearcherController {
  findStructureByVariableId(variableId: IRcodeVariableId): Promise<StructureInfo | undefined>;

  getStructureById(structureId: StructureId): Promise<Structure>;

  printStructure(structureId: StructureId): Promise<string>;

  printDataFlowForFunction(functionId: IRcodeFunctionId): Promise<string>;
}

export const getResearcherApi = () => {
  return window_.researcherApi as ResearcherController;
};
