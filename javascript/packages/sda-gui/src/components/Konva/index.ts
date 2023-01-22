import Konva from 'konva';
export * from './Stage';
export * from './ClippingGroup';
export * from './Block';

export const setCursor = <T>(e: Konva.KonvaEventObject<T>, cursor: string) => {
  const container = e.target.getStage()?.container();
  if (container) {
    container.style.cursor = cursor;
  }
};
