import { Platform, Register } from 'sda-core/platform';
import { PlatformX86 } from 'sda-platform-x86';

const platforms: { [name: string]: Platform } = {};

export const findPlatform = (name: string): Platform => {
  if (name in platforms) {
    return platforms[name];
  }
  throw new Error(`Platform ${name} not found`);
};

export const getPlatforms = (): Platform[] => {
  return Object.values(platforms);
};

export const addPlatform = (platform: Platform) => {
  platforms[platform.name] = platform;
};

export const initDefaultPlatforms = () => {
  addPlatform(PlatformX86.New(false));
  addPlatform(PlatformX86.New(true));

  const pl = findPlatform('x86');
  const repo = pl.registerRepository;
  const regId = repo.getRegisterId('rax');
  const type = repo.getRegisterType(regId);
  if (type === Register.Type.Generic) {
    console.log('rax is a generic register');
  }
  console.log('1) reg id = ', regId, ' type = ', Register.Type[type]);
};
