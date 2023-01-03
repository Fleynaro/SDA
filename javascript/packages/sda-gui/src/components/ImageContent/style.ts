export default {
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
        width: 100,
      },
      offset: {
        width: 50,
      },
      instruction: {
        width: 200,
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
