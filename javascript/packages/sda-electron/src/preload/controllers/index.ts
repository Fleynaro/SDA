import { contextBridge } from 'electron';
import EventController from './event-controller';
import WindowController from './window-controller';
import ProjectController from './project-controller';

const initControllers = () => {
    contextBridge.exposeInMainWorld('eventApi', EventController);
    contextBridge.exposeInMainWorld('windowApi', WindowController);
    contextBridge.exposeInMainWorld('projectApi', ProjectController);
};

export default initControllers;