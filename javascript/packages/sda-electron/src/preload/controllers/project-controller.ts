import { invokerFactory } from '../utils';
import { ProjectController } from '../../api/project';

const invoke = invokerFactory("Project");

const ProjectControllerImpl: ProjectController = {
    getProjects: () =>
        invoke("getProjects"),

    createProject: (path: string, platformName: string) =>
        invoke("createProject", path, platformName)
};

export default ProjectControllerImpl;