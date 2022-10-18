import { BrowserWindow, BrowserWindowConstructorOptions } from "electron";
import isElectronDev from 'electron-is-dev';
import * as path from "path";

const DefaultWindowOptions: BrowserWindowConstructorOptions = {
    webPreferences: {
        preload: path.join(__dirname, '../preload/index.js'),
        sandbox: false
    }
}

let Windows: BrowserWindow[] = [];

export const createWindow = (route: string, options: BrowserWindowConstructorOptions) => {
    const window = new BrowserWindow({
        ...DefaultWindowOptions,
        ...options
    });
    window.loadURL(
        isElectronDev
            ? `http://localhost:3000?route=${route}`
            : `file://${path.join(__dirname, '../build/index.html?route=${route}')}`
    );
    if (isElectronDev)
        window.webContents.openDevTools({ mode: 'detach' });
    Windows.push(window);
    return window;
}

export const notifyWindows = (event: string, ...args: any[]) => {
    Windows.forEach(window => window.webContents.send(event, ...args));
}