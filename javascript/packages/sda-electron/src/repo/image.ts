import { instance_of } from 'sda-bindings';
import {
  EventPipe,
  IRcodeContextSync,
  IRcodePcodeSync,
  IRcodeProgram,
  Image,
  ObjectAddedEvent,
  ObjectRemovedEvent,
  PcodeGraph,
  StandartSymbolTable,
} from 'sda-core';
import { findPlatform } from './platform';

interface ImageInfo {
  image: Image;
  pcodeGraph: PcodeGraph;
  ircodeProgram: IRcodeProgram;
  ctxSync: IRcodeContextSync;
  pcodeSync: IRcodePcodeSync;
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
        const ctxPipe = image.context.eventPipe;
        const symbolTable = StandartSymbolTable.New(image.context, `${image.id}-table`);
        const pcodeGraph = PcodeGraph.New(ctxPipe, image.context.platform);
        const ircodeProgram = IRcodeProgram.New(pcodeGraph, symbolTable);
        const callingConv = findPlatform('x86-64').callingConventions[0];
        const ctxSync = IRcodeContextSync.New(ircodeProgram, symbolTable, callingConv);
        const pcodeSync = IRcodePcodeSync.New(ircodeProgram);
        ctxPipe.connect(ctxSync.eventPipe);
        ctxPipe.connect(pcodeSync.eventPipe);
        images[image.hashId] = {
          image: image,
          pcodeGraph: pcodeGraph,
          ircodeProgram,
          ctxSync,
          pcodeSync,
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
