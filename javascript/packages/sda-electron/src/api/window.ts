import { ObjectId } from './common';

export enum WindowName {
    ProjectManager = "ProjectManager",
    Project = "Project"
}

export interface ProjectWindowPayload {
    projectId: ObjectId;
}

export interface WindowController {
    openProjectManagerWindow(): Promise<void>;

    openProjectWindow(payload: ProjectWindowPayload): Promise<void>;
}

export interface WindowInfo {
    name: WindowName;
    payload: any;
}
export interface WindowClientController extends WindowController {
    getWindowInfo(): Promise<WindowInfo>;
}

export const getWindowApi = (window: any) => {
    return window.windowApi as WindowClientController;
}