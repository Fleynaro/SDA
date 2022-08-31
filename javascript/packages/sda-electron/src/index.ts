import core from 'sda-core';

var ctx = new core.Context();
var dataType = new core.VoidDataType(ctx);
if (dataType.isVoid) {
    console.log("Void data type!!!");
    const obj = dataType.test((x: number) => {
        return x >= 5;
        });
    console.log("Void data type - end (result: " + obj.result + ")");
}