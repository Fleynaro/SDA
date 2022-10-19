import BaseController from './base-controller';
import { notifyWindows } from "../utils/window";
import { NotifierController } from '../api/notifier';
import { ObjectId } from '../api/common';
import { ObjectChangeType } from '../api/notifier';

class NotifierControllerImpl extends BaseController implements NotifierController {
    constructor() {
        super("Notifier");
        this.register("notifyAboutObjectChange", this.notifyAboutObjectChange);
    }

    public async notifyAboutObjectChange(id: ObjectId, changeType: ObjectChangeType): Promise<void> {
        notifyWindows(`Notifier.ObjectChange`, id, changeType);
    }

    public subscribeToObjectChanges(callback: (id: ObjectId, changeType: ObjectChangeType) => void): () => void {
        throw new Error("Method not implemented.");
    }
}

export default NotifierControllerImpl;