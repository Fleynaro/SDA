import ProjectController from './project-controller';
import NotifierController from './notifier-controller';
import { Program } from 'sda';

export let projectController: ProjectController;
export let notifierController: NotifierController;

export const initControllers = (program: Program) => {
    projectController = new ProjectController(program);
    notifierController = new NotifierController();
};