import { ImageAnalyser, PEImageAnalyser } from 'sda-core/image';

let analyser: { [name: string]: ImageAnalyser } = {};

export const findImageAnalyser = (name: string): ImageAnalyser => {
    if (name in analyser) {
        return analyser[name];
    }
    throw new Error(`Image analyser ${name} not found`);
}

export const getImageAnalysers = (): ImageAnalyser[] => {
    return Object.values(analyser);
}

export const addImageAnalyser = (imageAnalyser: ImageAnalyser) => {
    analyser[imageAnalyser.name] = imageAnalyser;
}

export const initDefaultImageAnalysers = () => {
    addImageAnalyser(PEImageAnalyser.New());
}