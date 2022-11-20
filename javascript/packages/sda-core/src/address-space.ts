import m from './module';
import { Context } from './context';
import { ContextObject } from './object';
import { Image } from './image';

export declare class AddressSpace extends ContextObject {
  images: Image[];

  static New(context: Context, name: string): AddressSpace;
}

module.exports = {
  ...module.exports,
  AddressSpace: m.AddressSpace,
};
