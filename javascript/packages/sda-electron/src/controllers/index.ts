import WindowController from './window-controller';
import ProjectController from './project-controller';
import PlatformController from './platform-controller';
import AddressSpaceController from './address-space-controller';
import ImageController from './image-controller';
import PcodeController from './p-code-controller';
import IRcodeController from './ir-code-controller';

export let windowController: WindowController;
export let projectController: ProjectController;
export let platformController: PlatformController;
export let addressSpaceController: AddressSpaceController;
export let imageController: ImageController;
export let pcodeController: PcodeController;
export let ircodeController: IRcodeController;

export const initControllers = () => {
  windowController = new WindowController();
  projectController = new ProjectController();
  platformController = new PlatformController();
  addressSpaceController = new AddressSpaceController();
  imageController = new ImageController();
  pcodeController = new PcodeController();
  ircodeController = new IRcodeController();
};
