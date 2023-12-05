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

export type IRcodeFunctionId = IRcodeObjectId;

export type IRcodeVariableId = {
  functionId: IRcodeObjectId;
  variableId: number;
};

export type IRcodeValueDto =
  | {
      type: 'variable';
      id: IRcodeVariableId;
      name: string;
    }
  | {
      // it's overlapped by PcodeVarnodeDto
      type: 'constant';
    }
  | {
      // it's overlapped by PcodeVarnodeDto
      type: 'register';
    };

export type IRcodeFunction = {
  id: IRcodeFunctionId;
};

export interface IRcodeController {
  getProgramIdByImage(imageId: ObjectId): Promise<ObjectId>;

  getFunctionAt(programId: ObjectId, entryOffset: Offset): Promise<IRcodeFunction | null>;

  getIRcodeTokenizedText(contextId: ObjectId, functionId: IRcodeFunctionId): Promise<TokenizedText>;
}

export const getIRcodeApi = () => {
  return window_.ircodeApi as IRcodeController;
};
