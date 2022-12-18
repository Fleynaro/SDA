import BaseController from './base-controller';
import { ImageController, Image as ImageDTO, ImageRow } from 'api/image';
import { Image, FileImageRW, SectionType, StandartSymbolTable } from 'sda-core';
import { GetOriginalInstructions } from 'sda';
import { ObjectId } from 'api/common';
import { toImageDTO, toImage, changeImage } from './dto/image';
import { toContext } from './dto/context';
import { findImageAnalyser } from 'repo/image-analyser';
import { binSearch } from 'utils/common';

interface ImageContent {
  rows: ImageRow[];
}

class ImageControllerImpl extends BaseController implements ImageController {
  private imageContents: Map<string, ImageContent>;

  constructor() {
    super('Image');
    this.register('getImage', this.getImage);
    this.register('createImage', this.createImage);
    this.register('changeImage', this.changeImage);
    this.register('getImageRowsAt', this.getImageRowsAt);
    this.imageContents = new Map();
  }

  public async getImage(id: ObjectId): Promise<ImageDTO> {
    const image = toImage(id);
    return toImageDTO(image);
  }

  public async createImage(
    contextId: ObjectId,
    name: string,
    analyserName: string,
    pathToImage: string,
  ): Promise<ImageDTO> {
    const imageRW = FileImageRW.New(pathToImage);
    imageRW.readFile();
    const context = toContext(contextId);
    const analyser = findImageAnalyser(analyserName);
    const globalSymbolTable = StandartSymbolTable.New(context, '');
    const image = Image.New(context, imageRW, analyser, name, globalSymbolTable);
    return toImageDTO(image);
  }

  public async changeImage(dto: ImageDTO): Promise<void> {
    changeImage(dto);
  }

  public async getImageRowsAt(id: ObjectId, offset: number, count: number): Promise<ImageRow[]> {
    let imageContent = this.imageContents.get(id.key);
    if (!imageContent) {
      const image = toImage(id);
      const imageSections = image.imageSections;
      const codeSection = imageSections.find((s) => s.type === SectionType.Code);
      if (!codeSection) {
        throw new Error('Image does not have code section');
      }
      const instructions = GetOriginalInstructions(image, codeSection);
      const rows: ImageRow[] = instructions.map((instr) => ({
        offset: instr.offset,
        length: instr.length,
      }));
      // TODO: const jumps =
      imageContent = { rows };
      this.imageContents.set(id.key, imageContent);
    }
    const rows = imageContent.rows;
    const foundRowIndex = binSearch(rows, (mid) => {
      const midRow = rows[mid];
      if (midRow.offset <= offset && offset < midRow.offset + midRow.length) return 0;
      return midRow.offset - offset;
    });
    if (foundRowIndex === -1) return [];
    return rows.slice(foundRowIndex, foundRowIndex + count);
  }
}

export default ImageControllerImpl;
