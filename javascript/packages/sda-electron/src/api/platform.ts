import { Identifiable, ObjectId, window_ } from './common';

export const PlatformClassName = 'Platform';

export interface Platform extends Identifiable {
  name: string;
}

export interface PlatformController {
  getPlatforms(): Promise<Platform[]>;
}

export const getPlatformApi = () => {
  return window_.platformApi as PlatformController;
};
