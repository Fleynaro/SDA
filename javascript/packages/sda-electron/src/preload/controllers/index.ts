import { contextBridge } from 'electron';
import EventController from './event-controller';
import WindowController from './window-controller';
import ProjectController from './project-controller';
import PlatformController from './platform-controller';
import AddressSpaceController from './address-space-controller';
import ImageController from './image-controller';
import PcodeController from './p-code-controller';
import IRcodeController from './ir-code-controller';

const initControllers = () => {
  contextBridge.exposeInMainWorld('eventApi', EventController);
  contextBridge.exposeInMainWorld('windowApi', WindowController);
  contextBridge.exposeInMainWorld('projectApi', ProjectController);
  contextBridge.exposeInMainWorld('platformApi', PlatformController);
  contextBridge.exposeInMainWorld('addressSpaceApi', AddressSpaceController);
  contextBridge.exposeInMainWorld('imageApi', ImageController);
  contextBridge.exposeInMainWorld('pcodeApi', PcodeController);
  contextBridge.exposeInMainWorld('ircodeApi', IRcodeController);
};

export default initControllers;
