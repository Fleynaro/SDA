declare module sda_core {
    abstract class ContextCallbacks {
        onObjectAdded(object: Object): void;
    }

    class ContextCallbacksImpl extends ContextCallbacks {
        static create(): ContextCallbacksImpl;

        onObjectAdded: (object: Object) => void;
    }

    class Context {
        static create(): Context;

        get callbacks(): ContextCallbacks;

        set callbacks(callbacks: ContextCallbacksImpl);
    }
}