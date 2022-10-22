import BaseController from './base-controller';
import { createWindow, BrowserWindowConstructorOptions } from "../utils/window";
import { windowOpenEmitter } from '../eventEmitter';
import { WindowName, WindowController, ProjectWindowPayload } from './../api/window';

class WindowControllerImpl extends BaseController implements WindowController {
    constructor() {
        super("Window");
        this.register("openProjectManagerWindow", this.openProjectManagerWindow);
        this.register("openProjectWindow", this.openProjectWindow);
    }

    public async openProjectManagerWindow(): Promise<void> {
        this.openWindow(WindowName.ProjectManager, {
            width: 800,
            height: 600,
        }, {});
    }

    public async openProjectWindow(payload: ProjectWindowPayload): Promise<void> {
        this.openWindow(WindowName.Project, {
            width: 800,
            height: 600,
        }, payload as any);
    }

    private openWindow(name: WindowName, options: BrowserWindowConstructorOptions, payload: any) {
        const window = createWindow(options);
        window.webContents.once('did-finish-load', () => {
            windowOpenEmitter(window)(name, payload);
        });
        return window;
    }
}

export default WindowControllerImpl;