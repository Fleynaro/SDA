import { Image } from 'sda-core/image';
import { Object } from 'sda-core/object';
import {
    Image as ImageDTO,
    ImageClassName
} from '../../api/image';
import { ObjectId } from '../../api/common';
import { toContextObjectDTO, changeContextObject } from './context-object';
import { toHash } from '../../utils/common';

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
        globalSymbolTableId: toImageId(image), // TODO: change!!!
    };
};

export const toImage = (id: ObjectId): Image => {
    const image = Object.Get(toHash(id)) as Image;
    if (!image) {
        throw new Error(`Image ${id.key} does not exist`);
    }
    return image;
}

export const changeImage = (dto: ImageDTO): void => {
    const image = toImage(dto.id);
    changeContextObject(image, dto);
}
