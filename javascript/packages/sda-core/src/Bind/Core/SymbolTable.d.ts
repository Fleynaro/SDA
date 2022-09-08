declare module sda {
    abstract class SymbolTable extends ContextObject {
        readonly usedSize: number;

        addSymbol(offset: number, symbol: Symbol): void;

        removeSymbol(offset: number): void;
    }

    class StandardSymbolTable extends SymbolTable {
        symbols: { [offset: number]: Symbol };

        static New(context: Context, name: string): StandardSymbolTable;
    }

    class OptimizedSymbolTable extends SymbolTable {
        symbolTables: StandardSymbolTable[];

        setFragmentsCount(count: number): void;

        static New(
            context: Context,
            name: string,
            minOffset: number,
            maxOffset: number,
            fragmentsCount: number): OptimizedSymbolTable;
    }
}