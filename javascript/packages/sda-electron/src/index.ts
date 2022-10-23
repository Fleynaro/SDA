import { app } from "electron";
import { windowController } from "./controllers";
import { initApp } from './app';

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

app.whenReady().then(async () => {
    initApp();
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