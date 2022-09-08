import core from "sda-core";
import platform from "sda-platform-x86";

const platformX86 = platform.PlatformX86.New(true);
const context = core.Context.New(platformX86);
console.log("Platform: " + platformX86.name);
console.log("CC: " + platformX86.callingConventions[0].name);

function test1() {
    const callbacks = core.ContextCallbacksImpl.New();
    callbacks.oldCallbacks = context.callbacks;
    callbacks.onObjectAdded = (obj) => {
        
        console.log("Object added (id = " + obj.id + ")");
        const data = obj.serialize();
        console.log("Serialized data:");
        console.log(data);

        if (obj instanceof core.ContextObject) {
            const ctxObj = obj as core.ContextObject;
            console.log("--- name: " + ctxObj.name);
        }
    };
    context.callbacks = callbacks;

    const dataType = core.VoidDataType.New(context);
    if (dataType.isVoid) {
        console.log("Void data type! (size: " + dataType.size + ")");
    }

    const ptrDt = dataType.getPointerTo();
    console.log("Pointer to void data type! (base: " + ptrDt.baseType.name + ")");

    //await new Promise(r => setTimeout(r, 5000));

    console.log('end');
}

function test2() {
    const enumDt = core.EnumDataType.New(context, "MyEnum", { 1: "One", 2: "Two" });
    console.log("Enum data type! (size: " + enumDt.size + ")");
    console.log("Fields:");
    for (const key in enumDt.fields) {
        console.log("--- " + enumDt.fields[key] + " = " + key);
    }
}

test2();