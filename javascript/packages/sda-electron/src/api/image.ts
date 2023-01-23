import { ObjectId, Offset, window_ } from './common';
import { ContextObject } from './context';
import { PcodeText } from './p-code';

export const ImageClassName = 'Image';

export interface ImageSection {
  name: string;
  minOffset: Offset;
  maxOffset: Offset;
  size: number;
}

export interface Image extends ContextObject {
  baseAddress: Offset;
  entryPointOffset: Offset;
  size: number;
  globalSymbolTableId: ObjectId;
  sections: ImageSection[];
}

export enum ImageRowType {
  Data,
  Instruction,
}

export interface ImageBaseRow {
  type: ImageRowType;
  offset: Offset;
  length: number;
}

export interface ImageInstructionRow extends ImageBaseRow {
  tokens: {
    type: string;
    text: string;
  }[];
  pcode?: PcodeText;
}

export interface ImageLoadRowOptions {
  tokens?: boolean;
  pcode?: boolean;
}

export interface Jump {
  from: Offset;
  to: Offset;
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

  getImageRowsAt(
    id: ObjectId,
    rowIdx: number,
    count: number,
    opts?: ImageLoadRowOptions,
  ): Promise<ImageBaseRow[]>;

  getImageTotalRowsCount(id: ObjectId): Promise<number>;

  offsetToRowIdx(id: ObjectId, offset: Offset): Promise<number>;

  getJumpsAt(id: ObjectId, startOffset: Offset, endOffset: Offset): Promise<Jump[][]>;

  analyzePcode(id: ObjectId, startOffsets: Offset[]): Promise<void>;
}

export const getImageApi = () => {
  return window_.imageApi as ImageController;
};
