import { AddressSpace } from 'sda-core/address-space';
import {
    AddressSpace as AddressSpaceDTO,
    AddressSpaceClassName
} from '../api/address-space';
import { ObjectId } from '../api/common';
import { toImageId } from './image';
import { toContextObjectDTO } from './context';

export const toAddressSpaceId = (addressSpace: AddressSpace): ObjectId => {
    return {
        key: addressSpace.hashId.toString(),
        className: AddressSpaceClassName,
    };
};

export const toAddressSpaceDTO = (addressSpace: AddressSpace): AddressSpaceDTO => {
    return {
        id: toAddressSpaceId(addressSpace),
        ...toContextObjectDTO(addressSpace),
        imageIds: addressSpace.images.map(toImageId),
    };
};
