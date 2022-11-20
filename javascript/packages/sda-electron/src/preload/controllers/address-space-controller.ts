import { invokerFactory } from '../utils';
import { AddressSpaceController, AddressSpace } from 'api/address-space';
import { ObjectId } from 'api/common';

const invoke = invokerFactory('AddressSpace');

const AddressSpaceControllerImpl: AddressSpaceController = {
  getAddressSpace: (id: ObjectId) => invoke('getAddressSpace', id),

  getAddressSpaces: (contextId: ObjectId) => invoke('getAddressSpaces', contextId),

  createAddressSpace: (contextId: ObjectId, name: string) =>
    invoke('createAddressSpace', contextId, name),

  changeAddressSpace: (dto: AddressSpace) => invoke('changeAddressSpace', dto),
};

export default AddressSpaceControllerImpl;
