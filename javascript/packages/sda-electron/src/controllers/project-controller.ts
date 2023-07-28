import { instance_of } from 'sda-bindings';
import BaseController from './base-controller';
import { toProject, toProjectDTO, toRecentProjectDTO } from './dto/project';
import { objectChangeEmitter, ObjectChangeType } from 'eventEmitter';
import {
  ProjectController,
  RecentProject as RecentProjectDTO,
  Project as ProjectDTO,
} from 'api/project';
import { ObjectId } from 'api/common';
import app from 'app';
import { loadJSON, saveJSON, doesFileExist, deleteFile } from 'utils/file';
import {
  Context,
  EventPipe,
  ObjectAddedEvent,
  ObjectModifiedEvent,
  ObjectRemovedEvent,
} from 'sda-core';
import { findPlatform } from 'repo/platform';
import { join as pathJoin } from 'path';
import { toId } from 'utils/common';
import isElectronDev from 'electron-is-dev';
import { createTestObjects } from 'test';

interface ProjectConfig {
  platformName: string;
}

const projectListFilePath = () => app.getUserPath('projects.json');
const projectConfigFilePath = (path: string) => pathJoin(path, 'project.json');

class ProjectControllerImpl extends BaseController implements ProjectController {
  constructor() {
    super('Project');
    this.register('getRecentProjects', this.getRecentProjects);
    this.register('updateRecentProjectsWithPath', this.updateRecentProjectsWithPath);
    this.register('getActiveProjects', this.getActiveProjects);
    this.register('getActiveProject', this.getActiveProject);
    this.register('createProject', this.createProject);
    this.register('openProject', this.openProject);
    this.register('deleteProject', this.deleteProject);
    this.register('saveProject', this.saveProject);
    this.register('canProjectBeSaved', this.canProjectBeSaved);
  }

  public async getRecentProjects(): Promise<RecentProjectDTO[]> {
    const file = projectListFilePath();
    if (!(await doesFileExist(file))) return [];
    const projects = await loadJSON<string[]>(file);
    return projects.map(toRecentProjectDTO);
  }

  private async saveRecentProjects(projects: RecentProjectDTO[]): Promise<void> {
    const file = projectListFilePath();
    await saveJSON(
      file,
      projects.map((p) => p.path),
    );
  }

  public async updateRecentProjectsWithPath(
    path: string,
    changeType: ObjectChangeType,
  ): Promise<void> {
    let recentProjects = await this.getRecentProjects();
    if (changeType == ObjectChangeType.Create && recentProjects.some((p) => p.path === path))
      return;
    const recentProject = toRecentProjectDTO(path);
    if (changeType == ObjectChangeType.Create) {
      recentProjects.push(recentProject);
    } else if (changeType == ObjectChangeType.Delete) {
      recentProjects = recentProjects.filter((p) => p.path !== path);
    }
    await this.saveRecentProjects(recentProjects);
    objectChangeEmitter()(recentProject.id, changeType);
  }

  public async getActiveProjects(): Promise<ProjectDTO[]> {
    return app.projects.map(toProjectDTO);
  }

  public async getActiveProject(id: ObjectId): Promise<ProjectDTO> {
    const project = toProject(id);
    return toProjectDTO(project);
  }

  public async openProject(path: string): Promise<ProjectDTO> {
    const config = await loadJSON<ProjectConfig>(projectConfigFilePath(path));
    const platform = findPlatform(config.platformName);
    const pipe = EventPipe.New('global');
    const context = Context.New(pipe, platform);
    pipe.subscribe((event) => {
      if (instance_of(event, ObjectAddedEvent)) {
        const e = event as ObjectAddedEvent;
        objectChangeEmitter()(toId(e.object), ObjectChangeType.Create);
      } else if (instance_of(event, ObjectModifiedEvent)) {
        const e = event as ObjectModifiedEvent;
        objectChangeEmitter()(toId(e.object), ObjectChangeType.Update);
      } else if (instance_of(event, ObjectRemovedEvent)) {
        const e = event as ObjectRemovedEvent;
        objectChangeEmitter()(toId(e.object), ObjectChangeType.Delete);
      }
    });
    if (isElectronDev) {
      createTestObjects(context);
    }
    const project = app.newProject(path, context);
    project.load();
    const projectDTO = toProjectDTO(project);
    await this.updateRecentProjectsWithPath(path, ObjectChangeType.Create);
    return projectDTO;
  }

  public async createProject(path: string, platformName: string): Promise<void> {
    if (!(await doesFileExist(path))) {
      throw new Error(`Path ${path} does not exist`);
    }
    const config: ProjectConfig = {
      platformName,
    };
    await saveJSON(projectConfigFilePath(path), config);
    await this.updateRecentProjectsWithPath(path, ObjectChangeType.Create);
  }

  public async deleteProject(path: string): Promise<void> {
    if (!(await doesFileExist(projectConfigFilePath(path)))) {
      throw new Error(`Project ${path} does not exist`);
    }
    await deleteFile(path);
    await this.updateRecentProjectsWithPath(path, ObjectChangeType.Delete);
  }

  public async saveProject(id: ObjectId): Promise<void> {
    const project = toProject(id);
    project.save();
  }

  public async canProjectBeSaved(id: ObjectId): Promise<boolean> {
    const project = toProject(id);
    return project.canBeSaved();
  }
}

export default ProjectControllerImpl;
