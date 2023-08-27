import Konva from 'konva';
import { TextConfig } from 'konva/lib/shapes/Text';
import { Block } from './Block';
import { Group, Rect, Text } from 'react-konva';
import { RenderProps } from './RenderBlock';
import { useCallback, useEffect } from 'react';
import { setCursor } from 'components/Konva';
import {
  KonvaTextSelectionType,
  toSelIndexFromRenderProps,
  useKonvaTextSelection,
} from './KonvaTextSelection';

export type TextStyleType = Omit<TextConfig, 'text'> & {
  fill?: string;
};

export type TextBlockProps = TextStyleType & {
  text: string;
  ctx?: {
    textStyle: TextStyleType;
    textSelection: KonvaTextSelectionType;
  };
};

export const StaticTextBlock = ({ text, ctx, ...propsStyle }: TextBlockProps) => {
  const selArea = ctx?.textSelection?.area || 'default';
  const style = { ...ctx?.textStyle, ...propsStyle };
  const konvaText = new Konva.Text(style);
  const textSize = konvaText.measureSize(text);
  const TextRender = (props: RenderProps) => {
    const {
      setSelectedArea,
      setFirstSelectedIdx,
      setLastSelectedIdx,
      selecting,
      setSelecting,
      selectionContains,
      addTokenToSelection,
    } = useKonvaTextSelection();
    const selIndex = toSelIndexFromRenderProps(props, ctx);
    const isSelected = selectionContains(selIndex, selArea);

    useEffect(() => {
      if (isSelected) {
        addTokenToSelection(selIndex, text);
        // TODO: you can also remove tokens from the selection here and no need to filter in the selectionMapToList
      }
    }, [isSelected]);

    const onMouseDown = useCallback(() => {
      setSelectedArea?.(selArea);
      setFirstSelectedIdx?.(selIndex);
      setLastSelectedIdx?.(undefined);
      setSelecting?.(true);
    }, [setFirstSelectedIdx, setSelecting]);

    const onMouseMove = useCallback(() => {
      if (selecting) {
        setLastSelectedIdx?.(selIndex);
      }
      // console.log('idx', ctx?.textSelection?.index, 'absX', props.absX, 'absY', props.absY);
    }, [selecting, setLastSelectedIdx]);

    const onMouseEnter = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      setCursor(e, 'text');
    }, []);

    const onMouseLeave = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      setCursor(e, 'default');
    }, []);

    return (
      <Group
        {...props}
        onMouseDown={onMouseDown}
        onMouseMove={onMouseMove}
        onMouseEnter={onMouseEnter}
        onMouseLeave={onMouseLeave}
      >
        {isSelected && <Rect width={props.width} height={props.height} fill="green"></Rect>}
        <Text text={text} {...style} />
      </Group>
    );
  };
  return <Block width={textSize.width} height={textSize.height} render={<TextRender />} />;
};
