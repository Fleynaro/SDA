import { ipcRenderer } from 'electron';
import { invokerFactory } from '../utils';
import { NotifierController, ObjectChangeType } from '../../api/notifier';
import { ObjectId } from '../../api/common';

const invoke = invokerFactory("Notifier");

const NotifierControllerImpl: NotifierController = {
    notifyAboutObjectChange: (id: ObjectId, changeType: ObjectChangeType) =>
        invoke("notifyAboutObjectChange", id, changeType),

    subscribeToObjectChanges: (callback: (id: ObjectId, changeType: ObjectChangeType) => void): () => void => {
        const channel = "Notifier.ObjectChange";
        const subscription = (event, id, changeType) => callback(id, changeType);
        ipcRenderer.on(channel, subscription);
        return () => ipcRenderer.removeListener(channel, subscription);
    }
};

export default NotifierControllerImpl;