import BaseController from './base-controller';
import { notifyAboutObjectChange } from '../notifier';
import { NotifierController } from '../api/notifier';
import { ObjectId } from '../api/common';
import { ObjectChangeType } from '../api/notifier';

class NotifierControllerImpl extends BaseController implements NotifierController {
    constructor() {
        super("Notifier");
        this.register("notifyAboutObjectChange", this.notifyAboutObjectChange);
    }

    public async notifyAboutObjectChange(id: ObjectId, changeType: ObjectChangeType): Promise<void> {
        notifyAboutObjectChange(id, changeType);
    }
}

export default NotifierControllerImpl;