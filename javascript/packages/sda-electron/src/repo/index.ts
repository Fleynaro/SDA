import { EventPipe } from 'sda-core';
import { initImageRepo } from './image';
import { initDefaultImageAnalyzers } from './image-analyser';
import { initDefaultPlatforms } from './platform';

export const initRepo = (eventPipe: EventPipe) => {
  initImageRepo(eventPipe);
  initDefaultPlatforms();
  initDefaultImageAnalyzers();
};
