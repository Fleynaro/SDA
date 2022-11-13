import BaseController from './base-controller';
import {
    ImageController,
    Image as ImageDTO,
    ImageAnalyser
} from '../api/image';
import { ObjectId } from '../api/common';
import { toImageDTO, toImage, changeImage } from './dto/image';

class ImageControllerImpl extends BaseController implements ImageController {

    constructor() {
        super("Image");
        this.register("getImage", this.getImage);
        this.register("createImage", this.createImage);
        this.register("changeImage", this.changeImage);
    }

    public async getImage(id: ObjectId): Promise<ImageDTO> {
        const image = toImage(id);
        return toImageDTO(image);
    }

    public async createImage(contextId: ObjectId, name: string, analyser: ImageAnalyser): Promise<ImageDTO> {
        throw new Error("Method not implemented.");
    }

    public async changeImage(dto: ImageDTO): Promise<void> {
        changeImage(dto);
    }
}

export default ImageControllerImpl;