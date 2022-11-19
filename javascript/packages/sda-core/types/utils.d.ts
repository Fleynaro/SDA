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

export abstract class AbstractPrinter {
    readonly output: string;

    flush(): void;

    setTabSize(tabSize: number): void;

    setParentPrinter(parent: AbstractPrinter): void;

    startBlock(): void;

    endBlock(): void;

    startCommenting(): void;

    endCommenting(): void;

    newLine(): void;
}