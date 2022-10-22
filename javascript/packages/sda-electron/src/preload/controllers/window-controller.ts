import { invokerFactory } from '../utils';
import { WindowController, ProjectWindowPayload } from '../../api/window';

const invoke = invokerFactory("Window");

const WindowControllerImpl: WindowController = {
    openProjectManagerWindow: () =>
        invoke("openProjectManagerWindow"),

    openProjectWindow: (payload: ProjectWindowPayload) =>
        invoke("openProjectWindow", payload)
};

export default WindowControllerImpl;