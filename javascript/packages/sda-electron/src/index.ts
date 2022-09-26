import { app, BrowserWindow, ipcMain } from "electron";
import * as path from "path";

// ------------------------------
import core from "sda-core";
import platform from "sda-platform-x86";
const platformX86 = platform.PlatformX86.New(true);
const context = core.Context.New(platformX86);
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
    const instrs = core.PcodeParser.Parse(pcodeStr, platformX86.registerRepository);
    const instr = instrs[0];
    console.log("Pcode instruction = " + instr.id);
    console.log("input0 = " + instr.input0.size + ", " + instr.input0.isRegister);

    const printer = core.PcodePrinter.New(platformX86.registerRepository);
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

    win.loadFile('index.html');
    win.webContents.openDevTools();
}

app.whenReady().then(() => {
    createWindow();
    
    test4();

    ipcMain.on('exec-message', (event, arg) => {
        event.returnValue = eval(arg);
    });

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0)
            createWindow();
    })
})

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin')
        app.quit()
})