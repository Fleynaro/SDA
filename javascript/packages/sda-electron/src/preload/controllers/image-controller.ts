import { invokerFactory } from '../utils';
import { ImageController, Image, ImageAnalyser } from '../../api/image';
import { ObjectId } from '../../api/common';

const invoke = invokerFactory("Image");

const ImageControllerImpl: ImageController = {
    getImage: (id: ObjectId) =>
        invoke("getImage", id),

    createImage: (contextId: ObjectId, name: string, analyser: ImageAnalyser) =>
        invoke("createImage", contextId, name, analyser),

    changeImage: (dto: Image) =>
        invoke("changeImage", dto),
};

export default ImageControllerImpl;