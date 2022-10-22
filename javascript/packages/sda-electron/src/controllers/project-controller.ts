import BaseController from './base-controller';
import { toProjectDTO } from '../dto/project';
import { objectChangeEmitter, ObjectChangeType } from '../eventEmitter';
import { ProjectController, Project as ProjectDTO } from '../api/project';
import { Program, Project } from 'sda';
import { Context } from 'sda-core/context';
import PlatformX86 from 'sda-platform-x86';

class ProjectControllerImpl extends BaseController implements ProjectController {
    private program: Program;

    constructor(program: Program) {
        super("Project");
        this.program = program;
        this.register("getProjects", this.getProjects);
        this.register("createProject", this.createProject);
    }

    public async getProjects(): Promise<ProjectDTO[]> {
        return this.program.projects.map(toProjectDTO);
    }

    public async createProject(path: string, platformName: string): Promise<ProjectDTO> {
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
        const projectDTO = toProjectDTO(project);
        objectChangeEmitter()(projectDTO.id, ObjectChangeType.Create);
        return projectDTO;
    }
}

export default ProjectControllerImpl;