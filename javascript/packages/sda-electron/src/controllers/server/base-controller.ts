import { ipcMain } from "electron";

abstract class BaseController {
    private controllerName: string;

    constructor(name: string) {
        this.controllerName = name;
    }

    protected register(methodName: string, handler: (...args: any[]) => any) {
        ipcMain.handle(this.controllerName + '.' + methodName, async (event, ...args) => {
            return await handler(...args);
        });
    }
}

export default BaseController;