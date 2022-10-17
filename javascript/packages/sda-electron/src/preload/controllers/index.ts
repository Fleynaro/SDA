import { contextBridge } from 'electron';
import ProjectController from './project-controller';

const initControllers = () => {
    contextBridge.exposeInMainWorld('projectApi', ProjectController);
};

export default initControllers;