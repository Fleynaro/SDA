import { contextBridge } from 'electron';
import ProjectController from './project-controller';
import NotifierController from './notifier-controller';

const initControllers = () => {
    contextBridge.exposeInMainWorld('projectApi', ProjectController);
    contextBridge.exposeInMainWorld('notifierApi', NotifierController);
};

export default initControllers;