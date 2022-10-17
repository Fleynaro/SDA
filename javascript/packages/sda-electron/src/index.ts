import { app } from "electron";
import { createWindow } from "./utils/window";
import initControllers from "./controllers";
import { Program } from 'sda';

app.whenReady().then(() => {
    createWindow("project", {
        width: 800,
        height: 600,
    });

    const program = Program.New();
    initControllers(program);

    // app.on('activate', () => {
    //     if (BrowserWindow.getAllWindows().length === 0)
    //         createWindow();
    // })
})

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin')
        app.quit()
})