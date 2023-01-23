import { StageSize, useStage } from 'components/Konva';
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
        width: 100,
      },
      instruction: {
        width: 'grow',
      },
    },
    instruction: {
      tokenColors: {
        ['Mneumonic']: '#eddaa4',
        ['Register']: '#93c5db',
        ['Number']: '#e8e8e8',
        ['AddressAbs']: '#d2b2d6',
        ['AddressRel']: '#bdbdbd',
      } as { [type: string]: string },
    },
    pcode: {
      tokenColors: {
        ['Mneumonic']: '#eddaa4',
        ['Register']: '#93c5db',
        ['VirtRegister']: '#93c5db',
        ['Number']: '#e8e8e8',
      } as { [type: string]: string },
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
    const stage = useStage();
    const resultStyles = useMemo(() => calcDynamicStyles(styles, stage.size), [styles, stage.size]);
    return { styles: resultStyles, setStyles };
  }, [styles]);

  return (
    <ImageContentStyleContext.Provider value={hook}>{children}</ImageContentStyleContext.Provider>
  );
};

export const ImageContentStyleBridgeProvider = ImageContentStyleContext.Provider;
export const ImageContentStyleBridgeConsumer = ImageContentStyleContext.Consumer;
