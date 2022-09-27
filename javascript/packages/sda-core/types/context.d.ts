import { Platform } from "./platform";

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
    callbacks: ContextCallbacks;

    static New(platform: Platform): Context;
}