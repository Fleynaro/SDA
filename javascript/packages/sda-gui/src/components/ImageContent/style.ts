const styles = {
  viewport: {
    scenePaddingRight: 10,
    sliderWidth: 15,
    avgRowHeight: 30,
    wheelDelta: 200,
    cacheSize: 200,
  },

  row: {
    padding: 5,
    cols: {
      jump: {
        // TODO: make this dynamic using useState, make this file contain default values
        width: 100,
      },
      offset: {
        width: 50,
      },
      instruction: {
        width: 300,
      },
    },
    instructionTokenColors: {
      ['Mneumonic']: '#eddaa4',
      ['Register']: '#93c5db',
      ['Number']: '#e8e8e8',
      ['AddressAbs']: '#d2b2d6',
      ['AddressRel']: '#bdbdbd',
    } as { [type: string]: string },
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

export default {
  ...styles,
  row: {
    ...styles.row,
    width: (stageWidth: number) =>
      stageWidth - styles.viewport.scenePaddingRight - styles.viewport.sliderWidth,
  },
};
