import { Context } from './context';
import { Lookupable } from './utils';

export interface Project extends Lookupable {
    path: string;
    context: Context;
}

export interface ProjectController {
    getProjects(): Promise<Project[]>;

    createProject(path: string, platformName: string): Promise<Project>;
}