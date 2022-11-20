import { invokerFactory } from '../utils';
import { PlatformController } from 'api/platform';

const invoke = invokerFactory('Platform');

const PlatformControllerImpl: PlatformController = {
  getPlatforms: () => invoke('getPlatforms'),
};

export default PlatformControllerImpl;
