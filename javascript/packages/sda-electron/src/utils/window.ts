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

export { BrowserWindow, BrowserWindowConstructorOptions }

export const createWindow = (options: BrowserWindowConstructorOptions) => {
    const window = new BrowserWindow({
        ...DefaultWindowOptions,
        ...options
    });
    window.loadURL(
        isElectronDev
            ? `http://localhost:3000`
            : `file://${path.join(__dirname, '../build/index.html')}`
    );
    if (isElectronDev)
        window.webContents.openDevTools({ mode: 'detach' });
    Windows.push(window);
    return window;
}

export const sendMessageToWindow = (window: BrowserWindow, event: string, ...args: any[]) =>
    window.webContents.send(event, ...args);

export const sendMessageToAllWindows = (event: string, ...args: any[]) =>
    Windows.forEach(window => sendMessageToWindow(window, event, ...args));