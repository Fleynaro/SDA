import { ObjectId, Offset, TokenGroupAction, TokenizedText, window_ } from './common';

export type ComplexOffset = number;

export const ParseComplexOffset = (offset: ComplexOffset) => ({
  byte: (offset >> 8) as Offset,
  index: offset & 0xff,
});

export enum PcodeTokenGroupAction {
  Instruction = 'instruction',
  Varnode = 'varnode',
  StructBlock = 'struct_block',
}

export interface PcodeLocatableByOffset {
  offset?: ComplexOffset;
  locatableByOffset: true;
}

export interface PcodeInstructionTokenGroupAction extends TokenGroupAction, PcodeLocatableByOffset {
  name: PcodeTokenGroupAction.Instruction;
  offset: ComplexOffset;
}

export interface PcodeStructBlockTokenGroupAction extends TokenGroupAction {
  name: PcodeTokenGroupAction.StructBlock;
  id: string;
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
