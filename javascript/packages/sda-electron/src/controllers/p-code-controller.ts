import BaseController from './base-controller';
import { ObjectId, Offset, TokenizedText } from 'api/common';
import {
  PcodeController,
  PcodeBlock as PcodeBlockDto,
  PcodeFunctionGraph as PcodeFunctionGraphDto,
  PcodeObjectId,
} from 'api/p-code';
import { toImage } from './dto/image';
import {
  pcodeStructTreeToTokenizedText,
  toFunctionGraphDto,
  toPcodeBlockDto,
  toPcodeGraph,
} from './dto/p-code';
import { getImageInfo } from 'repo/image';
import { toId } from 'utils/common';
import { PcodeStructTree } from 'sda-core';
import { toContext } from './dto/context';
import assert from 'assert';

class PcodeControllerImpl extends BaseController implements PcodeController {
  constructor() {
    super('Pcode');
    this.register('getGraphIdByImage', this.getGraphIdByImage);
    this.register('getFunctionGraphAt', this.getFunctionGraphAt);
    this.register('getBlockAt', this.getBlockAt);
    this.register('getPcodeTokenizedText', this.getPcodeTokenizedText);
  }

  public async getGraphIdByImage(imageId: ObjectId): Promise<ObjectId> {
    const image = toImage(imageId);
    const { pcodeGraph } = getImageInfo(image);
    return toId(pcodeGraph);
  }

  public async getBlockAt(
    graphId: ObjectId,
    offset: Offset,
    halfInterval: boolean,
  ): Promise<PcodeBlockDto | null> {
    const pcodeGraph = toPcodeGraph(graphId);
    const block = pcodeGraph.getBlockAt(offset, halfInterval);
    if (!block) return null;
    return toPcodeBlockDto(block);
  }

  public async getFunctionGraphAt(
    graphId: ObjectId,
    entryOffset: Offset,
  ): Promise<PcodeFunctionGraphDto | null> {
    const pcodeGraph = toPcodeGraph(graphId);
    const funcGraph = pcodeGraph.getFunctionGraphAt(entryOffset);
    if (!funcGraph) return null;
    return toFunctionGraphDto(funcGraph);
  }

  public async getPcodeTokenizedText(
    contextId: ObjectId,
    funcGraphId: PcodeObjectId,
  ): Promise<TokenizedText> {
    const context = toContext(contextId);
    const pcodeGraph = toPcodeGraph(funcGraphId.graphId);
    const funcGraph = pcodeGraph.getFunctionGraphAt(funcGraphId.offset);
    assert(funcGraph, `Function graph ${funcGraphId.offset} does not exist`);
    const structTree = PcodeStructTree.New();
    structTree.init(funcGraph);
    const { print } = pcodeStructTreeToTokenizedText(
      structTree,
      context.platform.registerRepository,
    );
    return print();
  }
}

export default PcodeControllerImpl;
