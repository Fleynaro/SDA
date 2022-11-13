import { AddressSpace } from 'sda-core/address-space';
import {
    AddressSpace as AddressSpaceDTO,
    AddressSpaceClassName
} from '../../api/address-space';
import { ObjectId } from '../../api/common';
import { changeContextObject, toContextObjectDTO } from './context-object';
import { toHash } from '../../utils/common';
import { toImage, toImageId } from './image';

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

export const toAddressSpace = (id: ObjectId): AddressSpace => {
    const addressSpace = AddressSpace.Get(toHash(id)) as AddressSpace; // TODO: remove cast (create Get static method for each class)
    if (!addressSpace) {
        throw new Error(`Address space ${id.key} does not exist`);
    }
    return addressSpace;
};

export const changeAddressSpace = (dto: AddressSpaceDTO): void => {
    const addressSpace = toAddressSpace(dto.id);
    changeContextObject(addressSpace, dto);
    addressSpace.images = dto.imageIds.map(toImage);
};
