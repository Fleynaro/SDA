import { ObjectId, window_ } from './common';
import { ContextObject } from './context';

export const AddressSpaceClassName = 'AddressSpace';

export interface AddressSpace extends ContextObject {
    imageIds: ObjectId[];
}

export interface AddressSpaceController {
    getAddressSpace(id: ObjectId): Promise<AddressSpace>;

    getAddressSpaces(contextId: ObjectId): Promise<AddressSpace[]>;
    
    createAddressSpace(contextId: ObjectId, name: string): Promise<AddressSpace>;
    
    changeAddressSpace(dto: AddressSpace): Promise<void>;
}

export const getAddressSpaceApi = () => {
    return window_.addressSpaceApi as AddressSpaceController;
}