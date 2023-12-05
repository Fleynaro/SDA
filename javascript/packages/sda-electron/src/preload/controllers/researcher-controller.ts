import { ResearcherController, StructureId } from 'api/researcher';
import { invokerFactory } from '../utils';
import { IRcodeFunctionId, IRcodeVariableId } from 'api/ir-code';

const invoke = invokerFactory('Researcher');

const ResearcherControllerImpl: ResearcherController = {
  findStructureByVariableId: (variableId: IRcodeVariableId) =>
    invoke('findStructureByVariableId', variableId),

  getStructureById: (structureId: StructureId) => invoke('getStructureById', structureId),

  printStructure: (structureId: StructureId) => invoke('printStructure', structureId),

  printDataFlowForFunction: (functionId: IRcodeFunctionId) =>
    invoke('printDataFlowForFunction', functionId),
};

export default ResearcherControllerImpl;
