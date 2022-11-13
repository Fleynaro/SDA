import { Platform } from 'sda-core/platform';
import {
    Platform as PlatformDTO,
    PlatformClassName
} from '../../api/platform';
import { ObjectId } from '../../api/common';

export const toPlatformId = (platform: Platform): ObjectId => {
    return {
        key: platform.hashId.toString(),
        className: PlatformClassName,
    };
};

export const toPlatformDTO = (platform: Platform): PlatformDTO => {
    return {
        id: toPlatformId(platform),
        name: platform.name,
    };
};