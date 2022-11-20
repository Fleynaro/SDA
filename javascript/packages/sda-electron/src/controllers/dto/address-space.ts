import { AddressSpace } from 'sda-core/address-space';
import { SdaObject } from 'sda-core/object';
import { AddressSpace as AddressSpaceDTO } from '../../api/address-space';
import { ObjectId } from '../../api/common';
import { changeContextObject, toContextObjectDTO } from './context-object';
import { toHash, toId } from '../../utils/common';
import { toImage } from './image';

export const toAddressSpaceDTO = (addressSpace: AddressSpace): AddressSpaceDTO => {
  return {
    ...toContextObjectDTO(addressSpace),
    imageIds: addressSpace.images.map(toId),
  };
};

export const toAddressSpace = (id: ObjectId): AddressSpace => {
  const addressSpace = SdaObject.Get(toHash(id)) as AddressSpace; // TODO: remove cast (create Get static method for each class)
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
