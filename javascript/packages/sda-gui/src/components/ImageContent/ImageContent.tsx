import { useState } from 'react';
import { ObjectId } from 'sda-electron/api/common';
import { getImageApi } from 'sda-electron/api/image';
import { useObject } from 'hooks';

export interface ImageContentProps {
  imageId: ObjectId;
}

export function ImageContent({ imageId }: ImageContentProps) {
  const image = useObject(() => getImageApi().getImage(imageId), [imageId]);
  return <>{image && image.name}</>;
}
