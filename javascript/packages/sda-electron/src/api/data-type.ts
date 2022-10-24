import { Identifiable, window_ } from './common';

export interface ContextObject extends Identifiable {
    name: string;
}

export interface DataType extends ContextObject {
    isVoid: boolean;
}

export interface DataTypeController {
    getDataTypeByName(name: string): DataType;
}

export const getDataTypeApi = () => {
    return window_.dataTypeApi as DataTypeController;
}