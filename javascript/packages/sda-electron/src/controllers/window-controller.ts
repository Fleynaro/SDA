import { IpcMainInvokeEvent } from 'electron';
import BaseController from './base-controller';
import { createWindow, BrowserWindowConstructorOptions } from "../utils/window";
import { WindowName, WindowInfo, WindowController, ProjectWindowPayload } from './../api/window';

class WindowControllerImpl extends BaseController implements WindowController {
    private windowInfos: Map<number, WindowInfo> = new Map();

    constructor() {
        super("Window");
        this.register("openProjectManagerWindow", this.openProjectManagerWindow);
        this.register("openProjectWindow", this.openProjectWindow);
        this.registerWithEvent("getWindowInfo", this.getWindowInfo);
    }

    public async openProjectManagerWindow(): Promise<void> {
        this.openWindow(WindowName.ProjectManager, {
            width: 450,
            height: 300,
            autoHideMenuBar: true,
            resizable: false
        }, {});
    }

    public async openProjectWindow(payload: ProjectWindowPayload): Promise<void> {
        this.windowInfos.forEach((info) => {
            if (info.name === WindowName.Project && info.payload.projectId === payload.projectId)
                throw new Error("Project window already opened");
        });
        this.openWindow(WindowName.Project, {
            width: 800,
            height: 600,
        }, payload as any);
    }

    public async getWindowInfo(event: IpcMainInvokeEvent): Promise<WindowInfo> {
        const windowId = event.sender.id;
        const info = this.windowInfos.get(windowId);
        if (!info) {
            throw new Error(`Window info not found for window with id ${windowId}`);
        }
        return info;
    }

    private openWindow(name: WindowName, options: BrowserWindowConstructorOptions, payload: any) {
        const window = createWindow(options);
        const windowId = window.webContents.id;
        window.on('closed', () => {
            this.windowInfos.delete(windowId);
        });
        this.windowInfos.set(windowId, {
            name,
            payload,
        });
        return window;
    }
}

export default WindowControllerImpl;