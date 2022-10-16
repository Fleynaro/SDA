import { Lookupable } from './utils';
 
export interface Project extends Lookupable {
    path: string;
}

export interface ProjectController {
    getProjects(): Promise<Project[]>;
}