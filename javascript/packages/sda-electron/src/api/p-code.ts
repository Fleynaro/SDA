import { Offset } from './common';

export type PcodeToken = {
  idx: number;
  type: string;
  text: string;
};

export type PcodeGroup = {
  start: number;
  end: number;
  action?:
    | {
        name: 'instruction';
        offset: Offset;
      }
    | {
        name: 'varnode';
      };
};

export type PcodeText = {
  tokens: PcodeToken[];
  groups: PcodeGroup[];
};
