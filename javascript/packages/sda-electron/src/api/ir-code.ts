import { ObjectId, Offset, TokenGroupAction, TokenizedText, window_ } from './common';
import { PcodeLocatableByOffset } from './p-code';

export enum IRcodeTokenGroupAction {
  Operation = 'ircode-operation',
  Value = 'ircode-value',
}

export interface IRcodeOperationTokenGroupAction extends TokenGroupAction, PcodeLocatableByOffset {
  name: IRcodeTokenGroupAction.Operation;
}

export interface IRcodeValueTokenGroupAction extends TokenGroupAction {
  name: IRcodeTokenGroupAction.Value;
  value: IRcodeValueDto;
}

export type IRcodeObjectId = {
  programId: ObjectId;
  offset: Offset;
};

export type IRcodeValueDto =
  | {
      type: 'variable';
      name: string;
    }
  | {
      type: 'constant';
      value: number;
    }
  | {
      type: 'register';
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
