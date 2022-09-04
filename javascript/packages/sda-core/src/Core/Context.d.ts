declare module sda_core {
    class ContextCallbacks {
        onObjectAdded: (obj: number) => void;
    }

    class Context {
        constructor();

        callbacks: ContextCallbacks;
    }
}