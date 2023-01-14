import Konva from 'konva';
export * from './Stage';
export * from './Block';
export * from './StaticText';
export * from './Text';
export * from './TextStyle';
export * from './ClippingGroup';
export * from './FormatText';

export const setCursor = <T>(e: Konva.KonvaEventObject<T>, cursor: string) => {
  const container = e.target.getStage()?.container();
  if (container) {
    container.style.cursor = cursor;
  }
};
