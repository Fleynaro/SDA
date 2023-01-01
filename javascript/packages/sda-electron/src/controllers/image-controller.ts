import BaseController from './base-controller';
import {
  ImageController,
  Image as ImageDTO,
  ImageBaseRow,
  ImageRowType,
  ImageInstructionRow,
} from 'api/image';
import { Image, FileImageRW, SectionType, StandartSymbolTable, Instruction } from 'sda-core';
import { GetOriginalInstructionInDetail, GetOriginalInstructions } from 'sda';
import { ObjectId } from 'api/common';
import { toImageDTO, toImage, changeImage } from './dto/image';
import { toContext } from './dto/context';
import { findImageAnalyser } from 'repo/image-analyser';
import { binSearch, toId } from 'utils/common';
import { JumpManager } from 'utils/instruction-jump';

interface ImageContent {
  baseRows: ImageBaseRow[];
  jumps: JumpManager;
}

class ImageControllerImpl extends BaseController implements ImageController {
  private imageContents: Map<string, ImageContent>;

  constructor() {
    super('Image');
    this.register('getImage', this.getImage);
    this.register('createImage', this.createImage);
    this.register('changeImage', this.changeImage);
    this.register('getImageRowsAt', this.getImageRowsAt);
    this.register('getImageTotalRowsCount', this.getImageTotalRowsCount);
    this.register('offsetToRowIdx', this.offsetToRowIdx);
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

  public async getImageRowsAt(
    id: ObjectId,
    rowIdx: number,
    count: number,
  ): Promise<ImageBaseRow[]> {
    const image = toImage(id);
    const imageContent = this.getImageContent(image);
    const slicedRows = imageContent.baseRows.slice(rowIdx, rowIdx + count);
    return slicedRows.map((r) => this.loadRow(image, r));
  }

  public async getImageTotalRowsCount(id: ObjectId): Promise<number> {
    const image = toImage(id);
    const imageContent = this.getImageContent(image);
    return imageContent.baseRows.length;
  }

  public async offsetToRowIdx(id: ObjectId, offset: number): Promise<number> {
    const image = toImage(id);
    const imageContent = this.getImageContent(image);
    const rows = imageContent.baseRows;
    const foundRowIndex = binSearch(rows, (mid) => {
      const midRow = rows[mid];
      if (midRow.offset <= offset && offset < midRow.offset + midRow.length) return 0;
      return midRow.offset - offset;
    });
    return foundRowIndex;
  }

  private getImageContent(image: Image): ImageContent {
    const imageId = toId(image);
    let imageContent = this.imageContents.get(imageId.key);
    if (imageContent) return imageContent;
    const imageSections = image.imageSections;
    const codeSection = imageSections.find((s) => s.type === SectionType.Code);
    if (!codeSection) {
      throw new Error('Image does not have code section');
    }
    const instructions = GetOriginalInstructions(image, codeSection);
    const baseRows: ImageBaseRow[] = instructions.map((instr) => ({
      type: ImageRowType.Instruction,
      offset: instr.offset,
      length: instr.length,
    }));
    const jumps = new JumpManager();
    imageContent = { baseRows, jumps };
    this.imageContents.set(imageId.key, imageContent);
    return imageContent;
  }

  private loadRow(image: Image, baseRow: ImageBaseRow): ImageBaseRow {
    if (baseRow.type === ImageRowType.Instruction) {
      const instruction = GetOriginalInstructionInDetail(image, baseRow.offset);
      return {
        ...baseRow,
        tokens: instruction.tokens.map((t) => ({
          type: Instruction.TokenType[t.type],
          text: t.text,
        })),
      } as ImageInstructionRow;
    }
    return baseRow;
  }
}

export default ImageControllerImpl;
