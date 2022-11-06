import ContextObjectController from './context-object-controller';
import {
    ImageController,
    Image as ImageDTO,
    ImageAnalyser
} from '../api/image';
import { ObjectId } from '../api/common';
import { toImageDTO } from '../dto/image';
import { toHash } from '../utils/common';
import { Image } from 'sda-core/image';

class ImageControllerImpl extends ContextObjectController implements ImageController {

    constructor() {
        super("Image");
        this.register("getImage", this.getImage);
        this.register("createImage", this.createImage);
        this.register("changeImage", this.changeImage);
    }
    // TODO: public to private
    public async getImage(id: ObjectId): Promise<ImageDTO> {
        const image = this.toImage(id);
        return toImageDTO(image); // TODO: move toImageDTO here (remove dto directory)
    }

    public async createImage(contextId: ObjectId, name: string, analyser: ImageAnalyser): Promise<ImageDTO> {
        throw new Error("Method not implemented.");
    }

    public async changeImage(dto: ImageDTO): Promise<void> {
        const image = this.toImage(dto.id);
        this.changeContextObject(image, dto);
    }

    public toImage(id: ObjectId): Image { // TODO: move toImage out of controller
        const image = Image.Get(toHash(id)) as Image;
        if (!image) {
            throw new Error(`Image ${id.key} does not exist`);
        }
        return image;
    }
}

export default ImageControllerImpl;