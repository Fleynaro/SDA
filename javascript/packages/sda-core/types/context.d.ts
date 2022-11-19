import { Object } from './object';
import { Platform } from "./platform";
import { AddressSpace } from "./address-space";
import { Hash, IIdentifiable } from './utils';

export abstract class ContextCallbacks {
    onObjectAdded(object: Object): void;

    onObjectModified(object: Object): void;

    onObjectRemoved(object: Object): void;
}

export class ContextCallbacksImpl extends ContextCallbacks {
    oldCallbacks: ContextCallbacks;

    onObjectAdded: (object: Object) => void;

    onObjectModified: (object: Object) => void;

    onObjectRemoved: (object: Object) => void;

    static New(): ContextCallbacksImpl;
}

export class Context implements IIdentifiable {
    readonly hashId: Hash;
    readonly className: string;
    callbacks: ContextCallbacks;
    addressSpaces: AddressSpace[];

    static New(platform: Platform): Context;

    static Get(hashId: Hash): Context;
}