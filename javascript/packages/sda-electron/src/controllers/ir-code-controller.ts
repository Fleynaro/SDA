import BaseController from './base-controller';
import { ObjectId, Offset, TokenizedText } from 'api/common';
import {
  IRcodeController,
  IRcodeFunction as IRcodeFunctionDto,
  IRcodeFunctionId,
} from 'api/ir-code';
import { toImage } from './dto/image';
import {
  ircodeStructTreeToTokenizedText,
  toIRcodeFunction,
  toIRcodeFunctionDto,
  toIRcodeProgram,
} from './dto/ir-code';
import { getImageInfo } from 'repo/image';
import { toId } from 'utils/common';
import { HolderSemantics, PcodeStructTree } from 'sda-core';
import { instance_of } from 'sda-bindings';

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
    imageId: ObjectId,
    functionId: IRcodeFunctionId,
  ): Promise<TokenizedText> {
    const image = toImage(imageId);
    const { researchers } = getImageInfo(image);
    const func = toIRcodeFunction(functionId);
    const structTree = PcodeStructTree.New();
    structTree.init(func.funcGraph);
    const { ircodePrinter, print } = ircodeStructTreeToTokenizedText(
      structTree,
      func,
      image.context.platform.registerRepository,
    );
    ircodePrinter.setOperationCommentProvider((operation) => {
      const semObj = researchers.semanticsRepo.getObject(operation.output);
      if (!semObj) return '';
      return semObj.semantics.map((s) => s.semantics.name).join(', ');
    });
    return print();
  }
}

export default IRcodeControllerImpl;
