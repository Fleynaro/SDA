// import { Context, VoidDataType } from 'sda-core';
// import core from 'sda-core';
// import f from './some'

// var ctx = new Context();

// class VoidDataTypeExt extends core.VoidDataType {
//     constructor(ctx: Context) {
//         super(ctx);
//     }

//     virtMethod2(x: number) {
//         console.log("Virtual method called with " + x);
//     }
// }

// var dataType = new VoidDataTypeExt(ctx);

// function test(dataType: VoidDataType) {
//     const callbacks = new core.ContextCallbacks();
//     callbacks.onObjectAdded = (obj: number) => {
//         console.log("Object added: " + obj);
//     };
//     ctx.callbacks = callbacks;

//     if (dataType.isVoid) {
//         console.log("Void data type!!!");
//         const obj = dataType.test((x: number) => {
//             return f(x);
//             });
//         console.log("Void data type - end (result: " + obj.result + ")");
//     }
// }

// test(dataType);

import { Context, ContextCallbacksImpl, VoidDataType } from "sda-core";

const context = Context.create();
const callbacks = ContextCallbacksImpl.create();
callbacks.onObjectAdded = (obj) => {
    console.log("Object added: " + obj.id);
};
context.callbacks = callbacks;

const dataType = VoidDataType.create(context);
if (dataType.isVoid) {
    console.log("Void data type!!! (size: " + dataType.size + ")");
}