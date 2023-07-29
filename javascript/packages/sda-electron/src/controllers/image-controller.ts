import BaseController from './base-controller';
import {
  ImageController,
  Image as ImageDTO,
  ImageBaseRow,
  ImageRowType,
  ImageInstructionRow,
  Jump as JumpDTO,
} from 'api/image';
import {
  Image,
  FileImageRW,
  SectionType,
  StandartSymbolTable,
  Instruction,
  toInstructionOffset,
  GetOriginalInstructionInDetail,
  GetOriginalInstructions,
  PcodeGraph,
} from 'sda-core';
import { ObjectId, Offset } from 'api/common';
import { ImageLoadRowOptions } from 'api/image';
import { toImageDTO, toImage, changeImage } from './dto/image';
import { toContext } from './dto/context';
import { toPcodeText } from './dto/p-code';
import { findImageAnalyser } from 'repo/image-analyser';
import { binSearch, toId } from 'utils/common';
import { Interval, Jump, JumpManager } from 'utils/instruction-jump';

interface ImageInfo {
  baseRows: ImageBaseRow[];
  jumps: JumpManager;
  pcodeGraph: PcodeGraph;
}

class ImageControllerImpl extends BaseController implements ImageController {
  private imageInfos: Map<string, ImageInfo>;

  constructor() {
    super('Image');
    this.register('getImage', this.getImage);
    this.register('createImage', this.createImage);
    this.register('changeImage', this.changeImage);
    this.register('getImageRowsAt', this.getImageRowsAt);
    this.register('getImageTotalRowsCount', this.getImageTotalRowsCount);
    this.register('offsetToRowIdx', this.offsetToRowIdx);
    this.register('getJumpsAt', this.getJumpsAt);
    this.register('analyzePcode', this.analyzePcode);
    this.imageInfos = new Map();
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
    opts?: ImageLoadRowOptions,
  ): Promise<ImageBaseRow[]> {
    const image = toImage(id);
    const info = this.getImageInfo(image);
    const slicedRows = info.baseRows.slice(rowIdx, rowIdx + count);
    return slicedRows.map((r) => this.loadRow(image, r, opts));
  }

  public async getImageTotalRowsCount(id: ObjectId): Promise<number> {
    const image = toImage(id);
    const info = this.getImageInfo(image);
    return info.baseRows.length;
  }

  public async offsetToRowIdx(id: ObjectId, offset: number): Promise<number> {
    const image = toImage(id);
    const info = this.getImageInfo(image);
    const rows = info.baseRows;
    const foundRowIndex = binSearch(rows, (mid) => {
      const midRow = rows[mid];
      if (midRow.offset <= offset && offset < midRow.offset + midRow.length) return 0;
      return midRow.offset - offset;
    });
    return foundRowIndex;
  }

  public async getJumpsAt(
    id: ObjectId,
    startOffset: Offset,
    endOffset: Offset,
  ): Promise<JumpDTO[][]> {
    const image = toImage(id);
    const info = this.getImageInfo(image);
    const foundJumps = info.jumps.findJumpsInInterval(new Interval(startOffset, endOffset));
    return foundJumps.map((jumpsOnLayer) =>
      jumpsOnLayer.map((jump) => ({ from: jump.offset, to: jump.targetOffset })),
    );
  }

  public async analyzePcode(id: ObjectId, startOffsets: Offset[]): Promise<void> {
    const image = toImage(id);
    const info = this.getImageInfo(image);
    const startInstructionOffsets = startOffsets.map(toInstructionOffset);
    for (const offset of startInstructionOffsets) {
      if (info.pcodeGraph.getInstructionAt(offset))
        throw new Error(`Instruction at ${offset} is already analyzed`);
      info.pcodeGraph.exploreImage(offset, image);
    }
  }

  private getImageInfo(image: Image): ImageInfo {
    const imageId = toId(image);
    let info = this.imageInfos.get(imageId.key);
    if (info) return info;
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
    for (const instr of instructions) {
      if (
        instr.type === Instruction.Type.UnconditionalBranch ||
        instr.type === Instruction.Type.ConditionalBranch
      ) {
        jumps.addJump(new Jump(instr.offset, instr.targetOffset));
      }
    }
    info = {
      baseRows,
      jumps,
      pcodeGraph: PcodeGraph.New(image.context.eventPipe, image.context.platform),
    };
    this.imageInfos.set(imageId.key, info);
    return info;
  }

  private loadRow(image: Image, baseRow: ImageBaseRow, opts?: ImageLoadRowOptions): ImageBaseRow {
    if (baseRow.type === ImageRowType.Instruction) {
      const row: ImageInstructionRow = {
        ...baseRow,
        tokens: [],
      };
      const offset = baseRow.offset;
      if (opts?.tokens) {
        const instruction = GetOriginalInstructionInDetail(image, offset);
        row.tokens = instruction.tokens.map((t) => ({
          type: Instruction.TokenType[t.type],
          text: t.text,
        }));
      }
      if (opts?.pcode) {
        const regRepo = image.context.platform.registerRepository;
        const imageInfo = this.getImageInfo(image);
        const pcodeInstructions = imageInfo.pcodeGraph.getInstructionsAt(offset);
        if (pcodeInstructions.length > 0) {
          row.pcode = toPcodeText(regRepo, pcodeInstructions);
        }
      }
      return row;
    }
    return baseRow;
  }
}

export default ImageControllerImpl;
