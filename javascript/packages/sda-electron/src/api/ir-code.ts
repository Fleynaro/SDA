import { ObjectId, Offset, TokenGroupAction, TokenizedText, window_ } from './common';

export enum IRcodeTokenGroupAction {
  Operation = 'ircode-operation',
  Value = 'ircode-value',
}

export interface IRcodeOperationTokenGroupAction extends TokenGroupAction {
  name: IRcodeTokenGroupAction.Operation;
}

export type IRcodeObjectId = {
  programId: ObjectId;
  offset: Offset;
};

export type IRcodeFunction = {
  id: IRcodeObjectId;
};

export interface IRcodeController {
  getProgramIdByImage(imageId: ObjectId): Promise<ObjectId>;

  getFunctionAt(programId: ObjectId, entryOffset: Offset): Promise<IRcodeFunction | null>;

  getIRcodeTokenizedText(contextId: ObjectId, functionId: IRcodeObjectId): Promise<TokenizedText>;
}

export const getIRcodeApi = () => {
  return window_.ircodeApi as IRcodeController;
};
