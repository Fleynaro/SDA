import BaseController from './base-controller';
import { ObjectId, Offset, TokenizedText } from 'api/common';
import {
  IRcodeController,
  IRcodeFunction as IRcodeFunctionDto,
  IRcodeFunctionId,
} from 'api/ir-code';
import { toImage } from './dto/image';
import { toContext } from './dto/context';
import {
  ircodeStructTreeToTokenizedText,
  toIRcodeFunction,
  toIRcodeFunctionDto,
  toIRcodeProgram,
} from './dto/ir-code';
import { getImageInfo } from 'repo/image';
import { toId } from 'utils/common';
import { PcodeStructTree } from 'sda-core';

class IRcodeControllerImpl extends BaseController implements IRcodeController {
  constructor() {
    super('IRcode');
    this.register('getProgramIdByImage', this.getProgramIdByImage);
    this.register('getFunctionAt', this.getFunctionAt);
    this.register('getIRcodeTokenizedText', this.getIRcodeTokenizedText);
  }

  public async getProgramIdByImage(imageId: ObjectId): Promise<ObjectId> {
    const image = toImage(imageId);
    const { ircodeProgram } = getImageInfo(image);
    return toId(ircodeProgram);
  }

  public async getFunctionAt(
    programId: ObjectId,
    entryOffset: Offset,
  ): Promise<IRcodeFunctionDto | null> {
    const program = toIRcodeProgram(programId);
    const func = program.getFunctionAt(entryOffset);
    if (!func) return null;
    return toIRcodeFunctionDto(func);
  }

  public async getIRcodeTokenizedText(
    contextId: ObjectId,
    functionId: IRcodeFunctionId,
  ): Promise<TokenizedText> {
    const context = toContext(contextId);
    const func = toIRcodeFunction(functionId);
    const structTree = PcodeStructTree.New();
    structTree.init(func.funcGraph);
    const text = ircodeStructTreeToTokenizedText(
      structTree,
      func,
      context.platform.registerRepository,
    );
    return text;
  }
}

export default IRcodeControllerImpl;
