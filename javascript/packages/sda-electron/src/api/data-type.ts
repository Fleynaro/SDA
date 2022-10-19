import { Identifiable } from './common';

export interface ContextObject extends Identifiable {
    name: string;
}

export interface DataType extends ContextObject {
    isVoid: boolean;
}

export interface DataTypeController {
    getDataTypeByName(name: string): DataType;
}

export const getDataTypeApi = (window: any) => {
    return window.dataTypeApi as DataTypeController;
}