import ContextObjectController from './context-object-controller';
import { imageController } from '../controllers';
import { toAddressSpaceDTO } from '../dto/address-space';
import {
    AddressSpaceController,
    AddressSpace as AddressSpaceDTO
} from '../api/address-space';
import { ObjectId } from '../api/common';
import { toHash } from '../utils/common';
import { AddressSpace } from 'sda-core/address-space';

class AddressSpaceControllerImpl extends ContextObjectController implements AddressSpaceController {

    constructor() {
        super("AddressSpace");
        this.register("getAddressSpace", this.getAddressSpace);
        this.register("getAddressSpaces", this.getAddressSpaces);
        this.register("createAddressSpace", this.createAddressSpace);
        this.register("changeAddressSpace", this.changeAddressSpace);
    }

    public async getAddressSpace(id: ObjectId): Promise<AddressSpaceDTO> {
        const addressSpace = this.toAddressSpace(id);
        return toAddressSpaceDTO(addressSpace);
    }

    public async getAddressSpaces(contextId: ObjectId): Promise<AddressSpaceDTO[]> {
        const context = this.toContext(contextId);
        return context.addressSpaces.map(toAddressSpaceDTO);
    }

    public async createAddressSpace(contextId: ObjectId, name: string): Promise<AddressSpaceDTO> {
        const context = this.toContext(contextId);
        const addressSpace = AddressSpace.New(context, name);
        return toAddressSpaceDTO(addressSpace);
    }

    public async changeAddressSpace(dto: AddressSpaceDTO): Promise<void> {
        const addressSpace = this.toAddressSpace(dto.id);
        this.changeContextObject(addressSpace, dto);
        addressSpace.images = dto.imageIds.map(imageController.toImage);
    }

    public toAddressSpace(id: ObjectId): AddressSpace {
        const addressSpace = AddressSpace.Get(toHash(id)) as AddressSpace; // TODO: remove cast (create Get static method for each class)
        if (!addressSpace) {
            throw new Error(`Address space ${id.key} does not exist`);
        }
        return addressSpace;
    }
}

export default AddressSpaceControllerImpl;