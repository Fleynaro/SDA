import { ObjectId, window_ } from './common';

export enum WindowName {
  ProjectManager = 'ProjectManager',
  Project = 'Project',
}

export interface ProjectWindowPayload {
  projectId: ObjectId;
}

export interface WindowController {
  openProjectManagerWindow(): Promise<void>;

  openProjectWindow(payload: ProjectWindowPayload): Promise<void>;

  openFilePickerDialog(directory: boolean, multiple: boolean): Promise<string[]>;
}

export interface WindowInfo {
  name: WindowName;
  payload: any;
}
export interface WindowClientController extends WindowController {
  getWindowInfo(): Promise<WindowInfo>;
}

export const getWindowApi = () => {
  return window_.windowApi as WindowClientController;
};
