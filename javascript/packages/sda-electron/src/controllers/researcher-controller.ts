import BaseController from './base-controller';
import { ResearcherController, Structure as StructureDto, StructureId } from 'api/researcher';
import { IRcodeFunctionId, IRcodeVariableId } from 'api/ir-code';
import { getImageInfoByProgram } from 'repo/image';
import { PrintDataFlowForFunction, PrintStructure } from 'sda-core';
import { toIRcodeFunction, toIRcodeProgram, toIRcodeVariable } from './dto/ir-code';
import { toStructure, toStructureDto } from './dto/researcher';

class ResearcherControllerImpl extends BaseController implements ResearcherController {
  constructor() {
    super('Researcher');
    this.register('findStructureByVariableId', this.findStructureByVariableId);
    this.register('getStructureById', this.getStructureById);
    this.register('printStructure', this.printStructure);
    this.register('printDataFlowForFunction', this.printDataFlowForFunction);
  }

  public async findStructureByVariableId(
    variableId: IRcodeVariableId,
  ): Promise<StructureDto | undefined> {
    const variable = toIRcodeVariable(variableId);
    const program = variable.sourceOperation.block.function.program;
    const { researchers } = getImageInfoByProgram(program);
    const dataFlowNode = researchers.dataFlowRepo.getNode(variable);
    if (!dataFlowNode) return undefined;
    const structure = researchers.structureRepo.getStructure(dataFlowNode);
    if (!structure) return undefined;
    return toStructureDto(program, structure);
  }

  public async getStructureById(structureId: StructureId): Promise<StructureDto> {
    const structure = toStructure(structureId);
    const program = toIRcodeProgram(structureId.programId);
    return toStructureDto(program, structure);
  }

  public async printStructure(structureId: StructureId): Promise<string> {
    const structure = toStructure(structureId);
    return PrintStructure(structure);
  }

  public async printDataFlowForFunction(functionId: IRcodeFunctionId): Promise<string> {
    const func = toIRcodeFunction(functionId);
    const program = toIRcodeProgram(functionId.programId);
    const { researchers } = getImageInfoByProgram(program);
    return PrintDataFlowForFunction(researchers.dataFlowRepo, func);
  }
}

export default ResearcherControllerImpl;
