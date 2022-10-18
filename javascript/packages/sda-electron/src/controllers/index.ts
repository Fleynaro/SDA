import BaseController from './base-controller';
import ProjectController from './project-controller';
import NotifierController from './notifier-controller';
import { Program } from 'sda';

let controllers: BaseController[] = [];

const initControllers = (program: Program) => {
    controllers = [
        new ProjectController(program),
        new NotifierController(),
    ];
};

export default initControllers;