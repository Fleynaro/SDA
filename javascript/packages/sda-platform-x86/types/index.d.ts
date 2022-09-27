import { Platform } from 'sda-core/platform';

export default class PlatformX86 extends Platform {
    take(): PlatformX86;

    static New(is64Version: boolean): PlatformX86;
}