import { app } from "electron";
import initServerControllers from "./controllers/server";
import { createWindow } from "./utils/window";

app.whenReady().then(() => {
    createWindow("project", {
        width: 800,
        height: 600,
    });

    initServerControllers();

    // app.on('activate', () => {
    //     if (BrowserWindow.getAllWindows().length === 0)
    //         createWindow();
    // })
})

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin')
        app.quit()
})