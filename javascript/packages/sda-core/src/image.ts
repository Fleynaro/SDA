import m from './module';
import { Context } from './context';
import { ContextObject } from './object';
import { SymbolTable } from './symbol-table';
import { Offset, ISerializable } from './utils';

export declare abstract class ImageRW {
  read(offset: Offset, size: number): Uint8Array;

  write(offset: Offset, data: Uint8Array): void;
}

export declare class VectorImageRW extends ImageRW {
  readonly size: number;
  readonly data: number[];

  static New(data: number[]): VectorImageRW;
}

export declare class FileImageRW extends ImageRW implements ISerializable {
  readonly size: number;

  readFile(): void;

  saveFile(): void;

  serialize(): object;

  deserialize(data: object): void;

  static New(pathToImgFile: string): FileImageRW;
}

export declare abstract class ImageAnalyser implements ISerializable {
  readonly name: string;
  baseAddress: Offset;
  entryPointOffset: Offset;
  imageSections: ImageSection[];

  analyse(): void;

  serialize(): object;

  deserialize(data: object): void;
}

export declare class PEImageAnalyser extends ImageAnalyser {
  static New(): PEImageAnalyser;
}

export declare class TestAnalyser extends ImageAnalyser {
  static New(): TestAnalyser;
}

export enum SectionType {
  None,
  Code,
  Data,
}

export declare class ImageSection {
  readonly name: string;
  readonly type: SectionType;
  readonly relVirtualAddress: number;
  readonly virtualSize: number;
  readonly pointerToRawData: number;
  readonly minOffset: Offset;
  readonly maxOffset: Offset;

  contains(offset: Offset): boolean;

  toOffset(imageFileOffset: Offset): Offset;

  toImageFileOffset(offset: Offset): Offset;
}

export declare class Image extends ContextObject {
  readonly rw: ImageRW;
  readonly baseAddress: Offset;
  readonly entryPointOffset: Offset;
  readonly imageSections: ImageSection[];
  readonly size: number;
  readonly globalSymbolTable: SymbolTable;
  //readonly pcodeGraph: PcodeGraph;

  contains(offset: Offset): boolean;

  toOffset(imageFileOffset: Offset): Offset;

  getImageSectionAt(offset: Offset): ImageSection;

  analyse(): void;

  static New(
    context: Context,
    rw: ImageRW,
    analyser: ImageAnalyser,
    name: string,
    globalSymbolTable: SymbolTable,
  ): Image;
}

module.exports = {
  ...module.exports,
  ImageRW: m.ImageRW,
  VectorImageRW: m.VectorImageRW,
  FileImageRW: m.FileImageRW,
  ImageAnalyser: m.ImageAnalyser,
  PEImageAnalyser: m.PEImageAnalyser,
  TestAnalyser: m.TestAnalyser,
  ImageSection: m.ImageSection,
  Image: m.Image,
};
