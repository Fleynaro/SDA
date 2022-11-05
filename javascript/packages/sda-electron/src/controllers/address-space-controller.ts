import BaseController from './base-controller';
import {
    AddressSpaceController,
    AddressSpace as AddressSpaceDTO
} from '../api/address-space';
import { ObjectId } from '../api/common';

class AddressSpaceControllerImpl extends BaseController implements AddressSpaceController {

    constructor() {
        super("AddressSpace");
        this.register("getAddressSpace", this.getAddressSpace);
        this.register("getAddressSpaces", this.getAddressSpaces);
        this.register("createAddressSpace", this.createAddressSpace);
        this.register("changeAddressSpace", this.changeAddressSpace);
    }

    public async getAddressSpace(id: ObjectId): Promise<AddressSpaceDTO> {
        throw new Error("Method not implemented.");
    }

    public async getAddressSpaces(contextId: ObjectId): Promise<AddressSpaceDTO[]> {
        throw new Error("Method not implemented.");
    }

    public async createAddressSpace(contextId: ObjectId, name: string): Promise<AddressSpaceDTO> {
        throw new Error("Method not implemented.");
    }

    public async changeAddressSpace(addressSpace: AddressSpaceDTO): Promise<void> {
        throw new Error("Method not implemented.");
    }
}

export default AddressSpaceControllerImpl;