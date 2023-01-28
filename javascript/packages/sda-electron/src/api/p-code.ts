import { Offset } from './common';

export type PcodeToken = {
  groupIdx: number;
  type: string;
  text: string;
};

export type PcodeGroup = {
  idx: number;
  action:
    | {
        name: 'root';
      }
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
