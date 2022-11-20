import { invokerFactory } from '../utils';
import { WindowClientController, ProjectWindowPayload } from 'api/window';

const invoke = invokerFactory('Window');

const WindowControllerImpl: WindowClientController = {
  openProjectManagerWindow: () => invoke('openProjectManagerWindow'),

  openProjectWindow: (payload: ProjectWindowPayload) => invoke('openProjectWindow', payload),

  openFilePickerDialog: (directory: boolean, multiple: boolean) =>
    invoke('openFilePickerDialog', directory, multiple),

  getWindowInfo: () => invoke('getWindowInfo'),
};

export default WindowControllerImpl;
