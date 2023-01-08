import { Offset } from './common';

export type PcodeToken =
  | {
      group: false;
      text: string;
      type: string;
    }
  | {
      group: true;
      tokens: PcodeToken[];
      action?:
        | {
            name: 'instruction';
            offset: Offset;
          }
        | {
            name: 'varnode';
          };
    };
