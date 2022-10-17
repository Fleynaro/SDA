import { Platform } from "./platform";
import { Hash } from './utils';

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

export class Context {
    readonly hashId: Hash;

    callbacks: ContextCallbacks;

    static New(platform: Platform): Context;

    static Get(hashId: Hash): Context;
}