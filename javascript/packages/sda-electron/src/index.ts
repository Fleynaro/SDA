import { Context, ContextCallbacksImpl, VoidDataType } from "sda-core";

const context = Context.create();
const callbacks = ContextCallbacksImpl.create();
callbacks.onObjectAdded = (obj) => {
    //oldCallbacks.onObjectAdded(obj);
    console.log("Object added: " + obj.id);
};
context.setCallbacks(callbacks);

const dataType = VoidDataType.create(context);
if (dataType.isVoid) {
    console.log("Void data type! (size: " + dataType.size + ")");
}