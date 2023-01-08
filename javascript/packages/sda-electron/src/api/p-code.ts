import { Offset } from './common';

export type PcodeNode =
  | {
      type: 'token';
      text: string;
      tokenType: string;
      idx: number;
    }
  | {
      type: 'group';
      childs: PcodeNode[];
      action?:
        | {
            name: 'instruction';
            offset: Offset;
          }
        | {
            name: 'varnode';
          };
    };
