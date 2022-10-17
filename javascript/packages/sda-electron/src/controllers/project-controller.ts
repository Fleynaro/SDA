import BaseController from './base-controller';
import { toProjectDTO } from './../dto/project';
import * as api from '@api/project';
import { Program, Project } from 'sda';
import { Context } from 'sda-core/context';
import PlatformX86 from 'sda-platform-x86';

class ProjectController extends BaseController implements api.ProjectController {
    private program: Program;

    constructor(program: Program) {
        super("Project");
        this.program = program;
        this.register("getProjects", this.getProjects);
        this.register("createProject", this.createProject);
    }

    public async getProjects(): Promise<api.Project[]> {
        return this.program.projects.map(toProjectDTO);
    }

    public async createProject(path: string, platformName: string): Promise<api.Project> {
        let platform;
        if (platformName === "x86") {
            platform = PlatformX86.New(false);
        } else if (platformName === "x86-64") {
            platform = PlatformX86.New(true);
        } else {
            throw new Error("Unknown platform");
        }
        const context = Context.New(platform);
        const project = Project.New(this.program, path, context);
        return toProjectDTO(project);
    }
}

export default ProjectController;