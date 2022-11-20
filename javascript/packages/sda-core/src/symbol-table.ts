import m from './module';
import { Context } from './context';
import { ContextObject } from './object';
import { Offset } from './utils';

export type SymbolInfo = {
  symbolTable: SymbolTable;
  symbolOffset: Offset;
  symbol: symbol;
};

export declare abstract class SymbolTable extends ContextObject {
  readonly usedSize: number;

  addSymbol(offset: Offset, symbol: symbol): void;

  removeSymbol(offset: Offset): void;

  getAllSymbols(): SymbolInfo[];

  getSymbolAt(offset: Offset): SymbolInfo;

  getAllSymbolsRecursivelyAt(offset: Offset): SymbolInfo[];
}

export declare class StandartSymbolTable extends SymbolTable {
  symbols: { [offset: Offset]: symbol };

  static New(context: Context, name: string): StandartSymbolTable;
}

export declare class OptimizedSymbolTable extends SymbolTable {
  symbolTables: StandartSymbolTable[];

  setFragmentsCount(count: number): void;

  static New(
    context: Context,
    name: string,
    minOffset: Offset,
    maxOffset: Offset,
    fragmentsCount: number,
  ): OptimizedSymbolTable;
}

module.exports = {
  ...module.exports,
  StandartSymbolTable: m.StandartSymbolTable,
  OptimizedSymbolTable: m.OptimizedSymbolTable,
};
