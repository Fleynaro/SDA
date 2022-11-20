import { dialog, IpcMainInvokeEvent } from 'electron';
import BaseController from './base-controller';
import { createWindow, BrowserWindowConstructorOptions } from 'utils/window';
import { WindowName, WindowInfo, WindowController, ProjectWindowPayload } from 'api/window';

class WindowControllerImpl extends BaseController implements WindowController {
  private windowInfos: Map<number, WindowInfo> = new Map();

  constructor() {
    super('Window');
    this.register('openProjectManagerWindow', this.openProjectManagerWindow);
    this.register('openProjectWindow', this.openProjectWindow);
    this.register('openFilePickerDialog', this.openFilePickerDialog);
    this.registerWithEvent('getWindowInfo', this.getWindowInfo);
  }

  public async openProjectManagerWindow(): Promise<void> {
    this.openWindow(
      WindowName.ProjectManager,
      {
        width: 700,
        height: 500,
        autoHideMenuBar: true,
        resizable: false,
      },
      {},
    );
  }

  public async openProjectWindow(payload: ProjectWindowPayload): Promise<void> {
    this.windowInfos.forEach((info) => {
      if (info.name === WindowName.Project && info.payload.projectId === payload.projectId)
        throw new Error('Project window already opened');
    });
    this.openWindow(
      WindowName.Project,
      {
        width: 800,
        height: 600,
        autoHideMenuBar: true,
      },
      payload as any,
    );
  }

  public async openFilePickerDialog(directory: boolean, multiple: boolean): Promise<string[]> {
    const options: Electron.OpenDialogOptions = {
      properties: [directory ? 'openDirectory' : 'openFile'],
    };
    if (multiple) {
      options.properties?.push('multiSelections');
    }
    const result = await dialog.showOpenDialog(options);
    return result.filePaths;
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
