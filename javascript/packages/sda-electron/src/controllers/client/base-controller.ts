import { contextBridge, ipcRenderer } from 'electron';

abstract class BaseController {
    private controllerName: string;

    constructor(name: string) {
        this.controllerName = name;
    }

    protected invoke(methodName: string, ...args: any[]) {
        return ipcRenderer.invoke(this.controllerName + '.' + methodName, ...args);
    }
}

export default BaseController;