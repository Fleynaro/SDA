import WindowController from './window-controller';
import ProjectController from './project-controller';
import VFileSystemController from './v-file-system-controller';
import PlatformController from './platform-controller';
import AddressSpaceController from './address-space-controller';
import ImageController from './image-controller';
import PcodeController from './p-code-controller';
import IRcodeController from './ir-code-controller';
import ResearcherController from './researcher-controller';

export let windowController: WindowController;
export let projectController: ProjectController;
export let vFileSystemController: VFileSystemController;
export let platformController: PlatformController;
export let addressSpaceController: AddressSpaceController;
export let imageController: ImageController;
export let pcodeController: PcodeController;
export let ircodeController: IRcodeController;
export let researcherController: ResearcherController;

export const initControllers = () => {
  windowController = new WindowController();
  projectController = new ProjectController();
  vFileSystemController = new VFileSystemController();
  platformController = new PlatformController();
  addressSpaceController = new AddressSpaceController();
  imageController = new ImageController();
  pcodeController = new PcodeController();
  ircodeController = new IRcodeController();
  researcherController = new ResearcherController();
};
