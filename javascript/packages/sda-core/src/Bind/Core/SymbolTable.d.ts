declare module sda {
    type SymbolInfo = {
        symbolTable: SymbolTable;
        symbolOffset: number;
        symbol: Symbol;
    }

    abstract class SymbolTable extends ContextObject {
        readonly usedSize: number;

        addSymbol(offset: number, symbol: Symbol): void;

        removeSymbol(offset: number): void;

        getAllSymbols(): SymbolInfo[];

        getSymbolAt(offset: number): SymbolInfo;

        getAllSymbolsRecursivelyAt(offset: number): SymbolInfo[];
    }

    class StandartSymbolTable extends SymbolTable {
        symbols: { [offset: number]: Symbol };

        static New(context: Context, name: string): StandartSymbolTable;
    }

    class OptimizedSymbolTable extends SymbolTable {
        symbolTables: StandartSymbolTable[];

        setFragmentsCount(count: number): void;

        static New(
            context: Context,
            name: string,
            minOffset: number,
            maxOffset: number,
            fragmentsCount: number): OptimizedSymbolTable;
    }
}