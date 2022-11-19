import BaseController from './base-controller';
import { toProject, toProjectDTO, toRecentProjectDTO } from './dto/project';
import { objectChangeEmitter, ObjectChangeType } from '../eventEmitter';
import {
    ProjectController,
    RecentProject as RecentProjectDTO,
    Project as ProjectDTO
} from '../api/project';
import { ObjectId } from '../api/common';
import { program, getUserPath } from '../app';
import { loadJSON, saveJSON, doesFileExist, deleteFile } from '../utils/file';
import { Project } from 'sda';
import { Context, ContextCallbacksImpl } from 'sda-core/context';
import { ContextObject } from 'sda-core/object';
import { findPlatform } from '../sda/platform';
import { join as pathJoin } from "path";

interface ProjectConfig {
    platformName: string;
}

const projectListFilePath = () => getUserPath('projects.json');
const projectConfigFilePath = (path: string) => pathJoin(path, 'project.json');

class ProjectControllerImpl extends BaseController implements ProjectController {

    constructor() {
        super("Project");
        this.register("getRecentProjects", this.getRecentProjects);
        this.register("updateRecentProjectsWithPath", this.updateRecentProjectsWithPath);
        this.register("getActiveProjects", this.getActiveProjects);
        this.register("getActiveProject", this.getActiveProject);
        this.register("createProject", this.createProject);
        this.register("openProject", this.openProject);
        this.register("deleteProject", this.deleteProject);
    }

    public async getRecentProjects(): Promise<RecentProjectDTO[]> {
        const file = projectListFilePath();
        if (!(await doesFileExist(file)))
            return [];
        const projects = await loadJSON<string[]>(file);
        return projects.map(toRecentProjectDTO);
    }

    private async saveRecentProjects(projects: RecentProjectDTO[]): Promise<void> {
        const file = projectListFilePath();
        await saveJSON(file, projects.map(p => p.path));
    }

    public async updateRecentProjectsWithPath(path: string, changeType: ObjectChangeType): Promise<void> {
        let recentProjects = await this.getRecentProjects();
        if (changeType == ObjectChangeType.Create && recentProjects.some(p => p.path === path))
            return;
        const recentProject = toRecentProjectDTO(path);
        if (changeType == ObjectChangeType.Create) {
            recentProjects.push(recentProject);
        } else if (changeType == ObjectChangeType.Delete) {
            recentProjects = recentProjects.filter(p => p.path !== path);
        }
        await this.saveRecentProjects(recentProjects);
        objectChangeEmitter()(recentProject.id, changeType);
    }

    public async getActiveProjects(): Promise<ProjectDTO[]> {
        return program.projects.map(toProjectDTO);
    }

    public async getActiveProject(id: ObjectId): Promise<ProjectDTO> {
        const project = toProject(id);
        return toProjectDTO(project);
    }

    public async openProject(path: string): Promise<ProjectDTO> {
        const config = await loadJSON<ProjectConfig>(projectConfigFilePath(path));
        const platform = findPlatform(config.platformName);
        const context = Context.New(platform);
        {
            const callbacks = ContextCallbacksImpl.New();
            callbacks.oldCallbacks = context.callbacks;
            callbacks.onObjectAdded = (obj) => {
                console.log("Object added (id = " + obj.id + ")");
                const data = obj.serialize();
                console.log("Serialized data:");
                console.log(data);

                if (obj instanceof ContextObject) {
                    const ctxObj = obj as ContextObject;
                    console.log("--- name: " + ctxObj.name);
                }
            };
            context.callbacks = callbacks;
        }
        const project = Project.New(program, path, context);
        const projectDTO = toProjectDTO(project);
        objectChangeEmitter()(projectDTO.id, ObjectChangeType.Create);
        await this.updateRecentProjectsWithPath(path, ObjectChangeType.Create);
        return projectDTO;
    }

    public async createProject(path: string, platformName: string): Promise<void> {
        const config: ProjectConfig = {
            platformName
        };
        await saveJSON(projectConfigFilePath(path), config);
        await this.updateRecentProjectsWithPath(path, ObjectChangeType.Create);
    }

    public async deleteProject(path: string): Promise<void> {
        if (!await doesFileExist(projectConfigFilePath(path))) {
            throw new Error(`Project ${path} does not exist`);
        }
        await deleteFile(path);
        await this.updateRecentProjectsWithPath(path, ObjectChangeType.Delete);
    }
}

export default ProjectControllerImpl;