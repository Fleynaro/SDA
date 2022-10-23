import WindowController from './window-controller';
import ProjectController from './project-controller';

export let windowController: WindowController;
export let projectController: ProjectController;

export const initControllers = () => {
    windowController = new WindowController();
    projectController = new ProjectController();
};