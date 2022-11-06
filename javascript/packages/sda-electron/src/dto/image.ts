import { Image } from 'sda-core/image';
import {
    Image as ImageDTO,
    ImageClassName
} from '../api/image';
import { ObjectId } from '../api/common';
import { toContextObjectDTO } from './context';

export const toImageId = (image: Image): ObjectId => {
    return {
        key: image.hashId.toString(),
        className: ImageClassName,
    };
};

export const toImageDTO = (image: Image): ImageDTO => {
    return {
        id: toImageId(image),
        ...toContextObjectDTO(image),
        baseAddress: image.baseAddress,
        entryPointOffset: image.entryPointOffset,
        size: image.size,
        globalSymbolTableId: toImageId(image),
    };
};
