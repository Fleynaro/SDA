import m from './module';
import { Context } from "./context";
import { ContextObject } from "./object";
import { DataType } from "./data-type";

export declare abstract class Symbol extends ContextObject {
    dataType: DataType;
}

export declare class VariableSymbol extends Symbol {
    static New(context: Context, name: string, dataType: DataType): VariableSymbol;
}

export declare class FunctionSymbol extends Symbol {
    static New(context: Context, name: string, dataType: DataType): FunctionSymbol;
}

export declare class FunctionParameterSymbol extends Symbol {
    static New(context: Context, name: string, dataType: DataType): FunctionParameterSymbol;
}

export declare class StructureFieldSymbol extends Symbol {
    static New(context: Context, name: string, dataType: DataType): StructureFieldSymbol;
}

module.exports = {
	...module.exports,
    VariableSymbol: m.VariableSymbol,
    FunctionSymbol: m.FunctionSymbol,
    FunctionParameterSymbol: m.FunctionParameterSymbol,
    StructureFieldSymbol: m.StructureFieldSymbol
};
