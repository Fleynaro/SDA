export type Hash = number;

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

  printTokenImpl: (text: string, token: AbstractPrinterToken) => void;
}
