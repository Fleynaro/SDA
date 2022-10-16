import BaseController from './base-controller';
import * as api from '@api/project';

class ProjectController extends BaseController implements api.ProjectController {
    constructor() {
        super("Project");
    }

    getProjects(): Promise<api.Project[]> {
        return this.invoke("getProjects");
    }
}

export default ProjectController;