import { Context, ContextCallbacks, ContextCallbacksImpl, VoidDataType } from "sda-core";

const context = Context.create();

const dataType222 = VoidDataType.create(context);
context.callbacks.onObjectAdded(dataType222);

const callbacks = ContextCallbacksImpl.create();
callbacks.onObjectAdded = (obj) => {
    //oldCallbacks.onObjectAdded(obj);
    console.log("Object added: " + obj.id);
    if (obj instanceof VoidDataType) {
        const voidDataType = obj as VoidDataType;
        console.log("Void data type added with size: " + voidDataType.size);
    }
};
context.callbacks = callbacks;

const dataType = VoidDataType.create(context);
if (dataType.isVoid) {
    console.log("Void data type! (size: " + dataType.size + ")");
}