import BaseController from './base-controller';
import { toPlatformDTO } from '../dto/platform';
import {
    PlatformController,
    Platform as PlatformDTO
} from '../api/platform';
import { getPlatforms } from '../sda/platform';

class PlatformControllerImpl extends BaseController implements PlatformController {

    constructor() {
        super("Platform");
        this.register("getPlatforms", this.getPlatforms);
    }

    public async getPlatforms(): Promise<PlatformDTO[]> {
        return getPlatforms().map(toPlatformDTO);
    }
}

export default PlatformControllerImpl;