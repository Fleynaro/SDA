import { Context, ContextCallbacksImpl, VoidDataType } from "sda-core";

const context = Context.create();
const callbacks = ContextCallbacksImpl.create();
callbacks.onObjectAdded = (obj) => {
    console.log("Object added: " + obj.id);
};
context.callbacks = callbacks;

const dataType = VoidDataType.create(context);
if (dataType.isVoid) {
    console.log("Void data type! (size: " + dataType.size + ")");
}

const otherDt = dataType.getType;
if (otherDt.isVoid) {
    console.log("Void data type!!! (size: " + otherDt.size + ")");
}