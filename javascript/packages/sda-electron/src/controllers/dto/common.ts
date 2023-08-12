import { TokenizedText, TokenGroupAction } from 'api/common';

export class TokenWriter {
  result: TokenizedText = {
    tokens: [],
    groups: [
      {
        idx: 0,
        action: {
          name: 'root',
        },
      },
    ],
  };

  curGroupIdx = 1;
  curGroup = this.result.groups[0];

  newToken(type: string, text: string): void {
    this.result.tokens.push({
      groupIdx: this.curGroup.idx,
      type,
      text,
    });
  }

  newGroup(action: TokenGroupAction, body?: () => void): void {
    const prevGroup = this.curGroup;
    const newGroup = {
      idx: this.curGroupIdx++,
      action,
    };
    this.result.groups.push(newGroup);
    this.curGroup = newGroup;
    body?.();
    this.curGroup = prevGroup;
  }
}
