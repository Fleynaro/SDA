import { ObjectId, Offset, window_ } from './common';
import { ContextObject } from './context';

export const ImageClassName = 'Image';

export interface Image extends ContextObject {
    baseAddress: Offset;
    entryPointOffset: Offset;
    size: number;
    globalSymbolTableId: ObjectId;
}

export enum ImageAnalyser {
    PE = 'PE'
}

export interface ImageController {
    getImage(id: ObjectId): Promise<Image>;
    
    createImage(contextId: ObjectId, name: string, analyser: ImageAnalyser): Promise<Image>;
    
    changeImage(dto: Image): Promise<void>;
}

export const getImageApi = () => {
    return window_.imageApi as ImageController;
}