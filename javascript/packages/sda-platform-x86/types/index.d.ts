import { Platform } from 'sda-core/platform';

export default class PlatformX86 extends Platform {
    static New(is64Version: boolean): PlatformX86;
}