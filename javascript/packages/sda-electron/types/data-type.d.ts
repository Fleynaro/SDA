import { Lookupable } from './utils';

export interface ContextObject extends Lookupable {
    name: string;
}

export interface DataType extends ContextObject {
    isVoid: boolean;
}

export interface DataTypeController {
    getDataTypeByName(name: string): DataType;
}