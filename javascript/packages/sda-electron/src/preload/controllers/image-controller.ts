import { invokerFactory } from '../utils';
import { ImageController, Image, ImageLoadRowOptions } from 'api/image';
import { ObjectId } from 'api/common';

const invoke = invokerFactory('Image');

const ImageControllerImpl: ImageController = {
  getImage: (id: ObjectId) => invoke('getImage', id),

  createImage: (contextId: ObjectId, name: string, analyserName: string, pathToImage: string) =>
    invoke('createImage', contextId, name, analyserName, pathToImage),

  changeImage: (dto: Image) => invoke('changeImage', dto),

  getImageRowsAt: (id: ObjectId, rowIdx: number, count: number, opts?: ImageLoadRowOptions) =>
    invoke('getImageRowsAt', id, rowIdx, count, opts),

  getImageTotalRowsCount: (id: ObjectId) => invoke('getImageTotalRowsCount', id),

  offsetToRowIdx: (id: ObjectId, offset: number) => invoke('offsetToRowIdx', id, offset),

  getJumpsAt: (id: ObjectId, startOffset: number, endOffset: number) =>
    invoke('getJumpsAt', id, startOffset, endOffset),

  analyzePcode: (id: ObjectId, startOffsets: number[]) => invoke('analyzePcode', id, startOffsets),
};

export default ImageControllerImpl;
