import { instance_of } from 'sda-bindings';
import { EventPipe, Image, ObjectAddedEvent, ObjectRemovedEvent, PcodeGraph } from 'sda-core';

interface ImageInfo {
  image: Image;
  pcodeGraph: PcodeGraph;
}

const images: { [id: number]: ImageInfo } = {};

export const getImageInfo = (image: Image): ImageInfo => {
  const info = images[image.hashId];
  if (info) {
    return info;
  }
  throw new Error(`Image ${image.name} not found`);
};

export const initImageRepo = (eventPipe: EventPipe) => {
  eventPipe.subscribe((event) => {
    if (instance_of(event, ObjectAddedEvent)) {
      const e = event as ObjectAddedEvent;
      if (instance_of(e.object, Image)) {
        const image = e.object as Image;
        images[image.hashId] = {
          image: image,
          pcodeGraph: PcodeGraph.New(image.context.eventPipe, image.context.platform),
        };
      }
    } else if (instance_of(event, ObjectRemovedEvent)) {
      const e = event as ObjectRemovedEvent;
      if (instance_of(e.object, Image)) {
        const image = e.object as Image;
        delete images[image.hashId];
      }
    }
  });
};
