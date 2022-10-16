import { ProjectController } from './project';
import { DataTypeController } from './data-type';

declare global {
    interface Window {
        projectApi: ProjectController;
        dataTypeApi: DataTypeController;
    }
}