import { Identifiable, ObjectChangeType, ObjectId, window_ } from './common';

export const ProjectClassName = 'Project';

export interface Project extends Identifiable {
    path: string;
    contextId: ObjectId;
}

export const RecentProjectClassName = 'RecentProject';

export interface RecentProject extends Identifiable {
    name: string,
    path: string;
}

export interface ProjectController {
    getRecentProjects(): Promise<RecentProject[]>;

    updateRecentProjectsWithPath(path: string, changeType: ObjectChangeType): Promise<void>;

    getActiveProjects(): Promise<Project[]>;

    getActiveProject(id: ObjectId): Promise<Project>;

    openProject(path: string): Promise<Project>;
    
    createProject(path: string, platformName: string): Promise<void>;

    deleteProject(path: string): Promise<void>;
}

export const getProjectApi = () => {
    return window_.projectApi as ProjectController;
}