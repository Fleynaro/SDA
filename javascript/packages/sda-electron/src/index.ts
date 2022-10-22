import { app } from "electron";
import { initControllers, windowController } from "./controllers";
import { Program } from 'sda';

/*
TODO: use aliases for imports (required: npm install tsc-alias)
"baseUrl": "./src",
"paths": {
    "@api/*": ["api/*"],
    "@dto/*": ["dto/*"],
    "@utils/*": ["utils/*"],
}

https://stackoverflow.com/questions/59179787/tsc-doesnt-compile-alias-paths
*/

app.whenReady().then(() => {
    const program = Program.New();
    initControllers(program);
    windowController.openProjectManagerWindow();

    // app.on('activate', () => {
    //     if (BrowserWindow.getAllWindows().length === 0)
    //         createWindow();
    // })
})

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin')
        app.quit()
})