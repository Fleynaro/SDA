import { Context, VoidDataType } from 'sda-core';
import core from 'sda-core';
import f from './some'

var ctx = new Context();

class VoidDataTypeExt extends core.VoidDataType {
    constructor(ctx: Context) {
        super(ctx);
    }

    virtMethod2(x: number) {
        console.log("Virtual method called with " + x);
    }
}

var dataType = new VoidDataTypeExt(ctx);

function test(dataType: VoidDataType) {
    if (dataType.isVoid) {
        dataType.virtMethod = (x: number) => {
            console.log("Virtual method called with " + x);
        };

        console.log("Void data type!!!");
        const obj = dataType.test((x: number) => {
            return f(x);
            });
        console.log("Void data type - end (result: " + obj.result + ")");
    }
}

test(dataType);