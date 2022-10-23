import { invokerFactory } from '../utils';
import { ProjectController } from '../../api/project';

const invoke = invokerFactory("Project");

const ProjectControllerImpl: ProjectController = {
    getRecentProjects: () =>
        invoke("getRecentProjects"),
    
    getActiveProjects: () =>
        invoke("getActiveProjects"),

    openProject: (path: string) =>
        invoke("openProject", path),

    createProject: (path: string, platformName: string) =>
        invoke("createProject", path, platformName)
};

export default ProjectControllerImpl;