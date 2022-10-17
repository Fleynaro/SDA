import { invokerFactory } from '../utils';
import * as api from '@api/project';

const invoke = invokerFactory("Project");

const ProjectController: api.ProjectController = {
    getProjects: () =>
        invoke("getProjects"),

    createProject: (path: string, platformName: string) =>
        invoke("createProject", path, platformName)
};

export default ProjectController;