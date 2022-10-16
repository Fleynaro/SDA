import { app, BrowserWindow, ipcMain } from "electron";
import isElectronDev from 'electron-is-dev';
import * as path from "path";

// ------------------------------
//import core from "sda-core";
import { Program } from "sda";
import { Context } from "sda-core/context";
import { PcodeParser, PcodePrinter } from "sda-core/p-code";
import { VoidDataType } from "sda-core/data-type";
import api from '../types/api';
import platform from "sda-platform-x86";
const platformX86 = platform.New(true);
const context = Context.New(platformX86);
console.log("Platform: " + platformX86.name);
const fastcallCC = platformX86.callingConventions[0];
console.log("CC: " + fastcallCC.name);
console.log("CC: " + fastcallCC.name);
console.log("reg = " + platformX86.registerRepository.getRegisterName(90));
function test4() {
    const pcodeStr = "\
        rcx:8 = COPY rcx:8 \
        rbx:8 = INT_MULT rdx:8, 4:8 \
        rbx:8 = INT_ADD rcx:8, rbx:8 \
        rbx:8 = INT_ADD rbx:8, 0x10:8 \
        STORE rbx:8, 1.0:8 \
    ";
    const instrs = PcodeParser.Parse(pcodeStr, platformX86.registerRepository);
    const instr = instrs[0];
    console.log("Pcode instruction = " + instr.id);
    console.log("input0 = " + instr.input0.size + ", " + instr.input0.isRegister);

    const printer = PcodePrinter.New(platformX86.registerRepository);
    for (const instr of instrs) {
        printer.flush();
        printer.printInstruction(instr);
        console.log(printer.output);
    }
}
// ------------------------------

const createWindow = () => {
    console.log("createWindow");
    const win = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js')
        }
    })

    win.loadURL(
        isElectronDev
            ? 'http://localhost:3000'
            : `file://${path.join(__dirname, '../build/index.html')}`
    );
    if (isElectronDev)
        win.webContents.openDevTools({ mode: 'detach' });
}

app.whenReady().then(() => {
    createWindow();
    
    //test4();

    function registerFunction<T extends Function>(name: string, method: T) {
        ipcMain.on(name, (event, ...args) => {
            event.returnValue = method(...args);
        });
    };

    function apiMethod(controllerName: string) {
        return <T extends Function>(methodName: string, method: T) => {
            registerFunction(controllerName + '.' + methodName, method);
        };
    }

    const register = apiMethod('dataTypeApi');
    register<api.DataTypeController['getDataTypeByName']>(
        'getDataTypeByName',
        (name) => {
            const dataType_ = VoidDataType.New(context);
            const dataType = VoidDataType.Get(dataType_.hashId);
            console.log("hash id = " + (dataType as any).hashId);
            return {
                name: dataType.name + name,
                isVoid: dataType.isVoid
            };
        }
    );

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0)
            createWindow();
    })
})

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin')
        app.quit()
})