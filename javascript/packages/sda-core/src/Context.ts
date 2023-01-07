import m from './module';
import { SdaObject } from './object';
import { Platform } from './platform';
import { AddressSpace } from './address-space';
import { Hash, IIdentifiable } from './utils';

export declare abstract class ContextCallbacks {
  readonly name: string;

  setPrevCallbacks(prevCallbacks: ContextCallbacks): void;

  setEnabled(enabled: boolean): void;

  onObjectAdded(object: SdaObject): void;

  onObjectModified(object: SdaObject): void;

  onObjectRemoved(object: SdaObject): void;

  static Find(name: string, callbacks: ContextCallbacks): ContextCallbacks | null;
}

export declare class ContextCallbacksImpl extends ContextCallbacks {
  onObjectAddedImpl: (object: SdaObject) => void;

  onObjectModifiedImpl: (object: SdaObject) => void;

  onObjectRemovedImpl: (object: SdaObject) => void;

  static New(): ContextCallbacksImpl;
}

export declare class Context implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly platform: Platform;
  callbacks: ContextCallbacks;
  addressSpaces: AddressSpace[];

  static New(platform: Platform): Context;

  static Get(hashId: Hash): Context;
}

module.exports = {
  ...module.exports,
  ContextCallbacks: m.ContextCallbacks,
  ContextCallbacksImpl: m.ContextCallbacksImpl,
  Context: m.Context,
};
