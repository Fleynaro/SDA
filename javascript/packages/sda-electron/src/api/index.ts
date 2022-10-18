import { ProjectController } from './project';
import { NotifierController } from './notifier';
import { DataTypeController } from './data-type';

declare global {
    interface Window {
        projectApi: ProjectController;
        notifierApi: NotifierController;
        dataTypeApi: DataTypeController;
    }
}