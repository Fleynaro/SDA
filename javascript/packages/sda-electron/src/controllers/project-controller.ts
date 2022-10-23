import BaseController from './base-controller';
import { toProjectDTO, toRecentProjectDTO } from '../dto/project';
import { objectChangeEmitter, ObjectChangeType } from '../eventEmitter';
import {
    ProjectController,
    RecentProject as RecentProjectDTO,
    Project as ProjectDTO
} from '../api/project';
import { program, getUserPath } from '../app';
import { loadJSON, saveJSON, doesFileExist } from '../utils/file';
import { Project } from 'sda';
import { Context } from 'sda-core/context';
import PlatformX86 from 'sda-platform-x86';
import { join as pathJoin } from "path";

interface ProjectConfig {
    platformName: string;
}

class ProjectControllerImpl extends BaseController implements ProjectController {

    constructor() {
        super("Project");
        this.register("getRecentProjects", this.getRecentProjects);
        this.register("getActiveProjects", this.getActiveProjects);
        this.register("createProject", this.createProject);
    }

    public async getRecentProjects(): Promise<RecentProjectDTO[]> {
        const file = getUserPath('projects.json');
        if (!(await doesFileExist(file)))
            return [];
        const projects = await loadJSON<string[]>(file);
        return projects.map(toRecentProjectDTO);
    }

    private async saveRecentProjects(projects: RecentProjectDTO[]): Promise<void> {
        const file = getUserPath('projects.json');
        await saveJSON(file, projects.map(p => p.path));
    }

    public async getActiveProjects(): Promise<ProjectDTO[]> {
        return program.projects.map(toProjectDTO);
    }

    public async openProject(path: string): Promise<ProjectDTO> {
        const config = await loadJSON<ProjectConfig>(pathJoin(path, 'project.json'));
        let platform;
        if (config.platformName === "x86") {
            platform = PlatformX86.New(false);
        } else if (config.platformName === "x86-64") {
            platform = PlatformX86.New(true);
        } else {
            throw new Error("Unknown platform");
        }
        const context = Context.New(platform);
        const project = Project.New(program, path, context);
        const projectDTO = toProjectDTO(project);
        objectChangeEmitter()(projectDTO.id, ObjectChangeType.Create);
        await this.updateRecentProjectsWithPath(path);
        return projectDTO;
    }

    public async createProject(path: string, platformName: string): Promise<void> {
        const config: ProjectConfig = {
            platformName
        };
        await saveJSON(pathJoin(path, 'project.json'), config);
        await this.updateRecentProjectsWithPath(path);
    }

    private async updateRecentProjectsWithPath(path: string): Promise<void> {
        const recentProjects = await this.getRecentProjects();
        if (!recentProjects.some(p => p.path === path)) {
            const recentProject = toRecentProjectDTO(path);
            recentProjects.push(recentProject);
            await this.saveRecentProjects(recentProjects);
            objectChangeEmitter()(recentProject.id, ObjectChangeType.Create);
        }
    }
}

export default ProjectControllerImpl;