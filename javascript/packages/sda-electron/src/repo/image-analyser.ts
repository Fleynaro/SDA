import { ImageAnalyser, PEImageAnalyser } from 'sda-core';

const analyzers: { [name: string]: ImageAnalyser } = {};

export const findImageAnalyser = (name: string): ImageAnalyser => {
  if (name in analyzers) {
    return analyzers[name];
  }
  throw new Error(`Image analyser ${name} not found`);
};

export const getImageAnalyzers = (): ImageAnalyser[] => {
  return Object.values(analyzers);
};

export const addImageAnalyser = (imageAnalyser: ImageAnalyser) => {
  analyzers[imageAnalyser.name] = imageAnalyser;
};

export const initDefaultImageAnalyzers = () => {
  addImageAnalyser(PEImageAnalyser.New());
};
