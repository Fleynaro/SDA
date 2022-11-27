import m from './module';
import { SdaObject } from './object';
import { Platform } from './platform';
import { AddressSpace } from './address-space';
import { Hash, IIdentifiable } from './utils';

export declare abstract class ContextCallbacks {
  onObjectAdded(object: SdaObject): void;

  onObjectModified(object: SdaObject): void;

  onObjectRemoved(object: SdaObject): void;
}

export declare class ContextCallbacksImpl extends ContextCallbacks {
  prevCallbacks: ContextCallbacks;

  onObjectAdded: (object: SdaObject) => void;

  onObjectModified: (object: SdaObject) => void;

  onObjectRemoved: (object: SdaObject) => void;

  static New(): ContextCallbacksImpl;
}

export declare class Context implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  callbacks: ContextCallbacks;
  addressSpaces: AddressSpace[];

  static New(platform: Platform): Context;

  static Get(hashId: Hash): Context;
}

module.exports = {
  ...module.exports,
  ContextCallbacksImpl: m.ContextCallbacksImpl,
  Context: m.Context,
};
