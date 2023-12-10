export type Hash = string;

export type Offset = number;

export interface IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
}

export interface ISerializable {
  serialize(): object;

  deserialize(data: object): void;
}

export enum AbstractPrinterToken {
  Symbol = 1,
  SpecSymbol = 2,
  Keyword = 3,
  Identifier = 4,
  Number = 5,
  String = 6,
  Comment = 7,
  Parent = 100,
}

export declare abstract class AbstractPrinter {
  readonly output: string;

  flush(): void;

  setTabSize(tabSize: number): void;

  setParentPrinter(parent: AbstractPrinter): void;

  startBlock(): void;

  endBlock(): void;

  startCommenting(): void;

  endCommenting(): void;

  newLine(): void;

  printToken(text: string, token: AbstractPrinterToken): void;

  // only for js printers
  printTokenImpl: (text: string, token: AbstractPrinterToken) => void;
}

export const StringToHash = (str: string): Hash => {
  let hash = 0;
  for (let i = 0; i < str.length; i++) {
    const char = str.charCodeAt(i);
    hash = (hash << 5) - hash + char;
    hash |= 0;
  }
  return hash.toString();
};
