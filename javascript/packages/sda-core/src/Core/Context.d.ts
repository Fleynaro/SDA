declare module sda_core {
    abstract class ContextCallbacks {
        onObjectAdded: (object: Object) => void;
    }

    class ContextCallbacksImpl extends ContextCallbacks {
        static create(): ContextCallbacksImpl;
    }

    class Context {
        static create(): Context;

        callbacks: ContextCallbacks;
    }
}