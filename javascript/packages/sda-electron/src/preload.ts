import { contextBridge, ipcRenderer } from 'electron';

contextBridge.exposeInMainWorld('testAPI', {
    exec: (code) => ipcRenderer.sendSync('exec-message', code)
})