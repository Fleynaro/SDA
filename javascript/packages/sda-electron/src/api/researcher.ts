import { ObjectId, window_ } from './common';
import { IRcodeFunctionId, IRcodeVariableId } from './ir-code';

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
  fields: { [offset: number]: StructureId };
  conditions: { [offset: number]: number[] };
  constants: { [offset: number]: number[] };
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
