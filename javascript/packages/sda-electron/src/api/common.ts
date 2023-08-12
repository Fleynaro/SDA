export type ObjectId = {
  key: string;
  className: string;
};

export interface Identifiable {
  id: ObjectId;
}

export const CmpObjectIds = (a: ObjectId, b: ObjectId) =>
  a.key === b.key && a.className === b.className;

export enum ObjectChangeType {
  Create,
  Update,
  Delete,
}

export type Offset = number;

export type Token = {
  groupIdx: number;
  type: string;
  text: string;
};

export interface TokenGroupAction {
  name: string;
}

export type TokenGroup = {
  idx: number;
  action: TokenGroupAction;
};

export type TokenizedText = {
  tokens: Token[];
  groups: TokenGroup[];
};

export let window_: any = null;

export const setWindow = (win: any) => {
  window_ = win;
};
