import BaseController from './base-controller';
import ProjectController from './project-controller';
import { Program } from 'sda';

let controllers: BaseController[] = [];

const initControllers = (program: Program) => {
    controllers = [
        new ProjectController(program)
    ];
};

export default initControllers;