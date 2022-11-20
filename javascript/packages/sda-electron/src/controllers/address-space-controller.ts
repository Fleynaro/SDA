import BaseController from './base-controller';
import { toAddressSpaceDTO, toAddressSpace, changeAddressSpace } from './dto/address-space';
import { AddressSpaceController, AddressSpace as AddressSpaceDTO } from 'api/address-space';
import { ObjectId } from 'api/common';
import { AddressSpace } from 'sda-core/address-space';
import { toContext } from './dto/context';

class AddressSpaceControllerImpl extends BaseController implements AddressSpaceController {
  constructor() {
    super('AddressSpace');
    this.register('getAddressSpace', this.getAddressSpace);
    this.register('getAddressSpaces', this.getAddressSpaces);
    this.register('createAddressSpace', this.createAddressSpace);
    this.register('changeAddressSpace', this.changeAddressSpace);
  }

  public async getAddressSpace(id: ObjectId): Promise<AddressSpaceDTO> {
    const addressSpace = toAddressSpace(id);
    return toAddressSpaceDTO(addressSpace);
  }

  public async getAddressSpaces(contextId: ObjectId): Promise<AddressSpaceDTO[]> {
    const context = toContext(contextId);
    return context.addressSpaces.map(toAddressSpaceDTO);
  }

  public async createAddressSpace(contextId: ObjectId, name: string): Promise<AddressSpaceDTO> {
    const context = toContext(contextId);
    const addressSpace = AddressSpace.New(context, name);
    return toAddressSpaceDTO(addressSpace);
  }

  public async changeAddressSpace(dto: AddressSpaceDTO): Promise<void> {
    changeAddressSpace(dto);
  }
}

export default AddressSpaceControllerImpl;
