import { ObjectId, Offset, TokenGroupAction, TokenizedText, window_ } from './common';

export enum PcodeTokenGroupAction {
  Instruction = 'instruction',
  Varnode = 'varnode',
}

export interface PcodeInstructionTokenGroupAction extends TokenGroupAction {
  name: PcodeTokenGroupAction.Instruction;
  offset: Offset;
}

export type PcodeObjectId = {
  graphId: ObjectId;
  offset: Offset;
};

export type PcodeBlock = {
  id: PcodeObjectId;
  functionGraph: PcodeFunctionGraph;
};

export type PcodeFunctionGraph = {
  id: PcodeObjectId;
};

export interface PcodeController {
  getGraphIdByImage(imageId: ObjectId): Promise<ObjectId>;

  getBlockAt(graphId: ObjectId, offset: Offset, halfInterval: boolean): Promise<PcodeBlock | null>;

  getFunctionGraphAt(graphId: ObjectId, entryOffset: Offset): Promise<PcodeFunctionGraph | null>;

  getPcodeTokenizedText(contextId: ObjectId, funcGraphId: PcodeObjectId): Promise<TokenizedText>;
}

export const getPcodeApi = () => {
  return window_.pcodeApi as PcodeController;
};
