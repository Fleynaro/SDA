import { StageSize, useKonvaStage } from 'components/Konva';
import { createContext, useState, useCallback, useMemo, useContext } from 'react';

const DefaultStyles = {
  viewport: {
    scenePaddingRight: 10,
    sliderWidth: 15,
    avgRowHeight: 30,
    wheelDelta: 200,
    cacheSize: 200,
  },

  row: {
    width: 0,
    padding: 5,
    cols: {
      jump: {
        width: 100,
      },
      offset: {
        width: 50,
      },
      instruction: {
        width: 0,
      },
    },
    instructionTokenColors: {
      ['Mneumonic']: '#eddaa4',
      ['Register']: '#93c5db',
      ['Number']: '#e8e8e8',
      ['AddressAbs']: '#d2b2d6',
      ['AddressRel']: '#bdbdbd',
    } as { [type: string]: string },
    pcode: {
      width: 100,
    },
  },

  jump: {
    arrowWidth: 2,
    pointerSize: 5,
    distanceBetweenLayers: 5,
    color: 'white',
    hoverColor: '#94d8ff',
    selectedColor: '#ed728a',
  },
};

export type StylesType = typeof DefaultStyles;

const calcDynamicStyles = (styles: StylesType, stageSize: StageSize) => {
  const rowWidth =
    stageSize.width - styles.viewport.scenePaddingRight - styles.viewport.sliderWidth;
  return {
    ...styles,
    row: {
      ...styles.row,
      width: rowWidth,
      cols: {
        ...styles.row.cols,
        instruction: {
          width: rowWidth - styles.row.cols.jump.width - styles.row.cols.offset.width,
        },
      },
    },
  };
};

type ImageContentStyleContextValue = () => {
  styles: StylesType;
  setStyles: (styles: StylesType) => void;
};

export const ImageContentStyleContext = createContext<ImageContentStyleContextValue | null>(null);

export const useImageContentStyle = () => {
  const hook = useContext(ImageContentStyleContext);
  if (!hook) throw new Error('ImageContentStyleContext is not set');
  return hook();
};

export const ImageContentStyleProvider = ({
  children,
  initStyles,
}: {
  children?: React.ReactNode;
  initStyles?: StylesType;
}) => {
  const [styles, setStyles] = useState<StylesType>({ ...DefaultStyles, ...initStyles });

  const hook = useCallback(() => {
    const stage = useKonvaStage();
    const resultStyles = useMemo(() => calcDynamicStyles(styles, stage.size), [styles, stage.size]);
    return { styles: resultStyles, setStyles };
  }, [styles]);

  return (
    <ImageContentStyleContext.Provider value={hook}>{children}</ImageContentStyleContext.Provider>
  );
};

export const ImageContentStyleBridgeProvider = ImageContentStyleContext.Provider;
export const ImageContentStyleBridgeConsumer = ImageContentStyleContext.Consumer;
