import BaseController from './base-controller';
import {
  ResearcherController,
  Semantics as SemanticsDto,
  SemanticsId,
  SemanticsObject as SemanticsObjectDto,
  Structure as StructureDto,
  StructureId,
  StructureLink,
} from 'api/researcher';
import { IRcodeFunctionId, IRcodeVariableId } from 'api/ir-code';
import { getImageInfoByProgram } from 'repo/image';
import { PrintDataFlowForFunction, PrintStructure } from 'sda-core';
import { toIRcodeFunction, toIRcodeProgram, toIRcodeVariable } from './dto/ir-code';
import {
  toSemantics,
  toSemanticsDto,
  toSemanticsObjectDto,
  toStructure,
  toStructureDto,
} from './dto/researcher';

class ResearcherControllerImpl extends BaseController implements ResearcherController {
  constructor() {
    super('Researcher');
    this.register('findStructureByVariableId', this.findStructureByVariableId);
    this.register('getStructureById', this.getStructureById);
    this.register('printStructure', this.printStructure);
    this.register('findSemanticsObjectByVariableId', this.findSemanticsObjectByVariableId);
    this.register('getSemanticsById', this.getSemanticsById);
    this.register('printDataFlowForFunction', this.printDataFlowForFunction);
  }

  public async findStructureByVariableId(
    variableId: IRcodeVariableId,
  ): Promise<StructureLink | undefined> {
    const variable = toIRcodeVariable(variableId);
    const program = variable.sourceOperation.block.function.program;
    const { researchers } = getImageInfoByProgram(program);
    const dataFlowNode = researchers.dataFlowRepo.getNode(variable);
    if (!dataFlowNode) return undefined;
    const link = researchers.structureRepo.getLink(dataFlowNode);
    if (!link) return undefined;
    const info = researchers.classRepo.getStructureInfo(link.structure);
    return {
      structure: toStructureDto(program, link.structure, info),
      offset: link.offset,
      own: link.own,
    };
  }

  public async getStructureById(structureId: StructureId): Promise<StructureDto> {
    const structure = toStructure(structureId);
    const program = toIRcodeProgram(structureId.programId);
    const { researchers } = getImageInfoByProgram(program);
    const info = researchers.classRepo.getStructureInfo(structure);
    return toStructureDto(program, structure, info);
  }

  public async printStructure(structureId: StructureId): Promise<string> {
    const structure = toStructure(structureId);
    return PrintStructure(structure);
  }

  public async findSemanticsObjectByVariableId(
    variableId: IRcodeVariableId,
  ): Promise<SemanticsObjectDto | undefined> {
    const variable = toIRcodeVariable(variableId);
    const program = variable.sourceOperation.block.function.program;
    const { researchers } = getImageInfoByProgram(program);
    const object = researchers.semanticsRepo.getObject(variable);
    if (!object) return undefined;
    return toSemanticsObjectDto(program, object);
  }

  public async getSemanticsById(semanticsId: SemanticsId): Promise<SemanticsDto> {
    const semantics = toSemantics(semanticsId);
    const program = toIRcodeProgram(semanticsId.programId);
    return toSemanticsDto(program, semantics);
  }

  public async printDataFlowForFunction(functionId: IRcodeFunctionId): Promise<string> {
    const func = toIRcodeFunction(functionId);
    const program = toIRcodeProgram(functionId.programId);
    const { researchers } = getImageInfoByProgram(program);
    return PrintDataFlowForFunction(researchers.dataFlowRepo, func);
  }
}

export default ResearcherControllerImpl;
