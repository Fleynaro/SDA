import { Context } from "./context";
import { ContextObject } from "./object";

export type SymbolInfo = {
    symbolTable: SymbolTable;
    symbolOffset: number;
    symbol: Symbol;
}

export abstract class SymbolTable extends ContextObject {
    readonly usedSize: number;

    addSymbol(offset: number, symbol: Symbol): void;

    removeSymbol(offset: number): void;

    getAllSymbols(): SymbolInfo[];

    getSymbolAt(offset: number): SymbolInfo;

    getAllSymbolsRecursivelyAt(offset: number): SymbolInfo[];
}

export class StandartSymbolTable extends SymbolTable {
    symbols: { [offset: number]: Symbol };

    static New(context: Context, name: string): StandartSymbolTable;
}

export class OptimizedSymbolTable extends SymbolTable {
    symbolTables: StandartSymbolTable[];

    setFragmentsCount(count: number): void;

    static New(
        context: Context,
        name: string,
        minOffset: number,
        maxOffset: number,
        fragmentsCount: number): OptimizedSymbolTable;
}