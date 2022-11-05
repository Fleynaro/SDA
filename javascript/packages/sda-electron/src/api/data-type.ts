import { window_ } from './common';
import { ContextObject } from './context';

export interface DataType extends ContextObject {
    isVoid: boolean;
}

export interface DataTypeController {
    getDataTypeByName(name: string): DataType;
}

export const getDataTypeApi = () => {
    return window_.dataTypeApi as DataTypeController;
}