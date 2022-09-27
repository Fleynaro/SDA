import { Context } from "./context";
import { ContextObject } from "./object";
import { DataType } from "./data-type";

export abstract class Symbol extends ContextObject {
    dataType: DataType;
}

export class VariableSymbol extends Symbol {
    static New(context: Context, name: string, dataType: DataType): VariableSymbol;
}

export class FunctionSymbol extends Symbol {
    static New(context: Context, name: string, dataType: DataType): FunctionSymbol;
}

export class FunctionParameterSymbol extends Symbol {
    static New(context: Context, name: string, dataType: DataType): FunctionParameterSymbol;
}

export class StructureFieldSymbol extends Symbol {
    static New(context: Context, name: string, dataType: DataType): StructureFieldSymbol;
}