declare module sda_core {
    abstract class ContextCallbacks {
        onObjectAdded(object: Object): void;

        onObjectModified(object: Object): void;

        onObjectRemoved(object: Object): void;
    }

    class ContextCallbacksImpl extends ContextCallbacks {
        oldCallbacks: ContextCallbacks;

        onObjectAdded: (object: Object) => void;

        onObjectModified: (object: Object) => void;

        onObjectRemoved: (object: Object) => void;

        static New(): ContextCallbacksImpl;
    }

    class Context {
        callbacks: ContextCallbacks;

        static New(): Context;
    }
}