import { Platform } from 'sda-core/platform';
import { Platform as PlatformDTO } from '../../api/platform';
import { toId } from '../../utils/common';

export const toPlatformDTO = (platform: Platform): PlatformDTO => {
  return {
    id: toId(platform),
    name: platform.name,
  };
};
