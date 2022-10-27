import { Platform } from 'sda-core/platform';
import PlatformX86 from 'sda-platform-x86';

let platforms: { [name: string]: Platform } = {};

export const findPlatform = (name: string): Platform => {
    if (name in platforms) {
        return platforms[name];
    }
    throw new Error(`Platform ${name} not found`);
}

export const getPlatforms = (): Platform[] => {
    return Object.values(platforms);
}

export const addPlatform = (platform: Platform) => {
    platforms[platform.name] = platform;
}

export const initDefaultPlatforms = () => {
    addPlatform(PlatformX86.New(false));
    addPlatform(PlatformX86.New(true));

    const pl = findPlatform('x86');
    const repo = pl.registerRepository;
    setTimeout(() => {
        console.log('1) reg id = ', repo.getRegisterId('rax'));
    }, 1000);
    setTimeout(() => {
        const repo = pl.registerRepository;
        console.log('2) reg id = ', repo.getRegisterId('rax'));
    }, 5000);
    setTimeout(() => {
        const repo = pl.registerRepository;
        console.log('3) reg id = ', repo.getRegisterId('rax'));
    }, 12000);
}