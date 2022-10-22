import WindowController from './window-controller';
import ProjectController from './project-controller';
import { Program } from 'sda';

export let windowController: WindowController;
export let projectController: ProjectController;

export const initControllers = (program: Program) => {
    windowController = new WindowController();
    projectController = new ProjectController(program);
};