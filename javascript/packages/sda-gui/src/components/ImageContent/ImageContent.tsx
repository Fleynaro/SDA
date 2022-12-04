import { useState } from 'react';
import { ObjectId } from 'sda-electron/api/common';
import { getImageApi } from 'sda-electron/api/image';
import { useObject } from 'hooks';
import { Layer, Rect, Text } from 'react-konva';
import { KonvaStage } from 'components/KonvaStage';

export interface ImageContentProps {
  imageId: ObjectId;
}

export function ImageContent({ imageId }: ImageContentProps) {
  const image = useObject(() => getImageApi().getImage(imageId), [imageId]);
  return (
    <>
      <KonvaStage sx={{ width: '100%', height: '100%' }}>
        <Layer>
          <Text text="Try to drag a star" />
          <Rect
            width={100}
            height={100}
            fill="red"
            shadowBlur={5}
            onClick={() => console.log('click')}
            draggable
          />
        </Layer>
      </KonvaStage>
    </>
  );
}
