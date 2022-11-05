import WindowController from './window-controller';
import ProjectController from './project-controller';
import PlatformController from './platform-controller';
import AddressSpaceController from './address-space-controller';

export let windowController: WindowController;
export let projectController: ProjectController;
export let platformController: PlatformController;
export let addressSpaceController: AddressSpaceController;

export const initControllers = () => {
    windowController = new WindowController();
    projectController = new ProjectController();
    platformController = new PlatformController();
    addressSpaceController = new AddressSpaceController();
};