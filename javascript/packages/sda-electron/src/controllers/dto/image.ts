import { SdaObject, Image, ImageSection } from 'sda-core';
import { Image as ImageDTO, ImageSection as ImageSectionDTO } from 'api/image';
import { ObjectId } from 'api/common';
import { toContextObjectDTO, changeContextObject } from './context-object';
import { toHash, toId } from 'utils/common';

export const toImageSectionDTO = (section: ImageSection): ImageSectionDTO => {
  return {
    name: section.name,
    minOffset: section.minOffset,
    maxOffset: section.maxOffset,
    size: section.maxOffset - section.minOffset,
  };
};

export const toImageDTO = (image: Image): ImageDTO => {
  return {
    ...toContextObjectDTO(image),
    baseAddress: image.baseAddress,
    entryPointOffset: image.entryPointOffset,
    size: image.size,
    globalSymbolTableId: toId(image.globalSymbolTable),
    sections: image.imageSections.map(toImageSectionDTO),
  };
};

export const toImage = (id: ObjectId): Image => {
  const image = SdaObject.Get(toHash(id)) as Image;
  if (!image) {
    throw new Error(`Image ${id.key} does not exist`);
  }
  return image;
};

export const changeImage = (dto: ImageDTO): void => {
  const image = toImage(dto.id);
  changeContextObject(image, dto);
};
