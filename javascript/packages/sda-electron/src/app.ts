import path from 'path';
import * as dotenv from 'dotenv';
dotenv.config({
  path: path.join(__dirname, '..', '.env'),
});
import { app } from 'electron';
import { existsSync, mkdirSync } from 'fs';
import { initRepo } from './repo';
import { initControllers } from './controllers';
import { objectChangeEmitter, ObjectChangeType } from './eventEmitter';
import { toId } from 'utils/common';
import { CleanUpSharedObjectLookupTable, Context, EventPipe, InitLogger } from 'sda-core';
import { Project } from 'project';

class App {
  readonly projects: Project[] = [];
  currentProject: Project | null = null;
  readonly eventPipe: EventPipe;

  constructor() {
    this.eventPipe = EventPipe.New('app');
  }

  init() {
    initRepo(this.eventPipe);
    initControllers();
    InitLogger();

    // create user directory if not exists
    const userDir = this.getUserDir();
    if (!existsSync(userDir)) {
      mkdirSync(userDir);
    }

    setInterval(() => {
      // TODO: remove when app exit
      CleanUpSharedObjectLookupTable();
    }, 10000);
  }

  newProject(path: string, context: Context) {
    const project = new Project(path, context);
    this.projects.push(project);
    objectChangeEmitter()(toId(project), ObjectChangeType.Create);
    return project;
  }

  deleteProject(project: Project) {
    const index = this.projects.indexOf(project);
    if (index >= 0) {
      objectChangeEmitter()(toId(project), ObjectChangeType.Delete);
      this.projects.splice(index, 1);
      project.destroy();
    }
  }

  getUserDir() {
    return path.join(app.getPath('documents'), 'SDA');
  }

  getUserPath(file: string) {
    return path.join(this.getUserDir(), file);
  }
}

export default new App();
