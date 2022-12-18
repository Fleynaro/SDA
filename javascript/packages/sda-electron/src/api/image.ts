import { ObjectId, Offset, window_ } from './common';
import { ContextObject } from './context';

export const ImageClassName = 'Image';

export interface ImageSection {
  name: string;
  minOffset: Offset;
  maxOffset: Offset;
}

export interface Image extends ContextObject {
  baseAddress: Offset;
  entryPointOffset: Offset;
  size: number;
  globalSymbolTableId: ObjectId;
  sections: ImageSection[];
}

export interface ImageRow {
  offset: Offset;
  length: number;
}

export interface ImageController {
  getImage(id: ObjectId): Promise<Image>;

  createImage(
    contextId: ObjectId,
    name: string,
    analyserName: string,
    pathToImage: string,
  ): Promise<Image>;

  changeImage(dto: Image): Promise<void>;

  getImageRowsAt(id: ObjectId, offset: Offset, count: number): Promise<ImageRow[]>;
}

export const getImageApi = () => {
  return window_.imageApi as ImageController;
};
