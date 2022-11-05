import { Image } from 'sda-core/image';
import {
    Image as ImageDTO,
    ImageClassName
} from '../api/address-space';
import { ObjectId } from '../api/common';
import { toContextId } from './context';

export const toImageId = (Image: Image): ObjectId => {
    return {
        key: Image.hashId.toString(),
        className: ImageClassName,
    };
};

export const toImageDTO = (Image: Image): ImageDTO => {
    return {
        id: toImageId(Image),
        images: Image.images.map(image => image.hashId.toString()),
    };
};
