import BaseController from './base-controller';
import { ImageController, Image as ImageDTO } from '../api/image';
import { Image, FileImageRW } from 'sda-core/image';
import { StandartSymbolTable } from 'sda-core/symbol-table';
import { ObjectId } from '../api/common';
import { toImageDTO, toImage, changeImage } from './dto/image';
import { toContext } from './dto/context';
import { findImageAnalyser } from '../sda/image-analyser';

class ImageControllerImpl extends BaseController implements ImageController {
  constructor() {
    super('Image');
    this.register('getImage', this.getImage);
    this.register('createImage', this.createImage);
    this.register('changeImage', this.changeImage);
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
}

export default ImageControllerImpl;
