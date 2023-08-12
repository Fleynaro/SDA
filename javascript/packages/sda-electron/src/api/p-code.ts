import { Offset, TokenGroupAction } from './common';

export enum PcodeTokenGroupAction {
  Instruction = 'instruction',
  Varnode = 'varnode',
}

export interface PcodeInstructionTokenGroupAction extends TokenGroupAction {
  name: PcodeTokenGroupAction.Instruction;
  offset: Offset;
}
