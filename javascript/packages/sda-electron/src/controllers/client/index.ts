import ProjectController from './project-controller';
import { contextBridge } from 'electron';

const initClientControllers = () => {
    contextBridge.exposeInMainWorld('projectApi', new ProjectController());
};

export default initClientControllers;