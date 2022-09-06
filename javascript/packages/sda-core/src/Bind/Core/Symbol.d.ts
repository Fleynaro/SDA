declare module sda_core {
    abstract class Symbol extends ContextObject {
        dataType: DataType;
    }

    class VariableSymbol extends Symbol {
        static New(context: Context, name: string, dataType: DataType): VariableSymbol;
    }

    class FunctionSymbol extends Symbol {
        static New(context: Context, name: string, dataType: DataType): FunctionSymbol;
    }

    class FunctionParameterSymbol extends Symbol {
        static New(context: Context, name: string, dataType: DataType): FunctionParameterSymbol;
    }

    class StructureFieldSymbol extends Symbol {
        static New(context: Context, name: string, dataType: DataType): StructureFieldSymbol;
    }
}