import m from './module';
import { Object } from './object';
import { Platform } from "./platform";
import { AddressSpace } from "./address-space";
import { Hash, IIdentifiable } from './utils';

export declare abstract class ContextCallbacks {
    onObjectAdded(object: Object): void;

    onObjectModified(object: Object): void;

    onObjectRemoved(object: Object): void;
}

export declare class ContextCallbacksImpl extends ContextCallbacks {
    oldCallbacks: ContextCallbacks;

    onObjectAdded: (object: Object) => void;

    onObjectModified: (object: Object) => void;

    onObjectRemoved: (object: Object) => void;

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
    Context: m.Context
};
