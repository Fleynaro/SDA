import BaseController from './base-controller';
import * as api from '@api/project';

class ProjectController extends BaseController implements api.ProjectController {
    constructor() {
        super("Project");
        this.register("getProjects", this.getProjects);
    }

    async getProjects(): Promise<api.Project[]> {
        return [];
    }
}

export default ProjectController;