import { AddressSpace } from 'sda-core/';
import {
    AddressSpace as AddressSpaceDTO,
    AddressSpaceClassName
} from '../api/address-space';
import { ObjectId } from '../api/common';
import { toContextId } from './context';
import { basename as pathBasename } from 'path';

export const toAddressSpaceId = (addressSpace: AddressSpace): ObjectId => {
    return {
        key: addressSpace.hashId.toString(),
        className: AddressSpaceClassName,
    };
};

export const toAddressSpaceDTO = (addressSpace: AddressSpace): AddressSpaceDTO => {
    return {
        id: toAddressSpaceId(addressSpace),
        contextId: toContextId(addressSpace.context),
    };
};
