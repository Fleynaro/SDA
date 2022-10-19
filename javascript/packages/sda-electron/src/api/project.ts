import { Identifiable, ObjectId } from './common';

export interface Project extends Identifiable {
    path: string;
    context: ObjectId;
}

export interface ProjectController {
    getProjects(): Promise<Project[]>;

    createProject(path: string, platformName: string): Promise<Project>;
}

export const getProjectApi = (window: any) => {
    return window.projectApi as ProjectController;
}