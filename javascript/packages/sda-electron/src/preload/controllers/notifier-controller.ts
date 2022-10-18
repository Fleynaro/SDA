import { invokerFactory } from '../utils';
import { NotifierController, ObjectChangeType } from '../../api/notifier';
import { ObjectId } from '../../api/common';

const invoke = invokerFactory("Notifier");

const NotifierControllerImpl: NotifierController = {
    notifyAboutObjectChange: (id: ObjectId, changeType: ObjectChangeType) =>
        invoke("notifyAboutObjectChange", id, changeType)
};

export default NotifierControllerImpl;