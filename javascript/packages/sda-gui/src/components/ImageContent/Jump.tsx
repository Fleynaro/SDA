import { useCallback } from 'react';
import { Jump } from 'sda-electron/api/image';
import { Arrow } from 'react-konva';
import Konva from 'konva';
import { setCursor } from 'components/Konva';
import style from './style'; // static style
import { useImageContentContext } from './context';

export const buildJump = (
  jump: Jump,
  layerLevel: number,
  fromY?: number,
  toY?: number,
): JSX.Element => {
  const direction = jump.from < jump.to ? 1 : -1;
  const startY = fromY || -10000 * direction; // 10000 = endless
  const endY = toY || 10000 * direction;
  const isClickable = fromY === undefined || toY === undefined;
  const startX = style.row.cols.jump.width;
  const layerOffset = style.jump.pointerSize + layerLevel * style.jump.distanceBetweenLayers;
  const p1 = [startX + style.jump.pointerSize, startY];
  const p2 = [startX - layerOffset, p1[1]];
  const p3 = [p2[0], endY];
  const p4 = [p1[0], p3[1]];
  function Elem() {
    const { selectedJump, setSelectedJump, goToOffset } = useImageContentContext();
    const isSelected = jump.from === selectedJump?.from && jump.to === selectedJump?.to;
    const color = isSelected ? style.jump.selectedColor : style.jump.color;

    const onClickJump = useCallback(async () => {
      let success = false;
      if (toY === undefined) {
        success = await goToOffset(jump.to);
      }
      if (fromY === undefined && !success) {
        success = await goToOffset(jump.from);
      }
      if (success) {
        setSelectedJump?.(jump);
        setTimeout(() => {
          setSelectedJump?.(undefined);
        }, 1500);
      }
    }, []);

    const onMouseEnter = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      const target = e.target as Konva.Arrow;
      target.fill(style.jump.hoverColor);
      target.stroke(style.jump.hoverColor);
      if (isClickable) setCursor(e, 'pointer');
    }, []);

    const onMouseLeave = useCallback((e: Konva.KonvaEventObject<MouseEvent>) => {
      const target = e.target as Konva.Arrow;
      if (target.fill() === style.jump.hoverColor) {
        target.fill(style.jump.color);
        target.stroke(style.jump.color);
      }
      if (isClickable) setCursor(e, 'default');
    }, []);

    return (
      <Arrow
        points={[p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], p4[0], p4[1]]}
        pointerLength={style.jump.pointerSize}
        pointerWidth={style.jump.pointerSize}
        fill={color}
        stroke={color}
        strokeWidth={style.jump.arrowWidth}
        onClick={onClickJump}
        onMouseEnter={onMouseEnter}
        onMouseLeave={onMouseLeave}
      />
    );
  }
  return <Elem />;
};
