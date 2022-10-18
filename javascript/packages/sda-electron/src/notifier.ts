import { notifyWindows } from "./utils/window";
import { ObjectChangeType } from "./api/notifier";
import { ObjectId } from "./api/common";

export { ObjectChangeType };

export const notifyAboutObjectChange = (id: ObjectId, changeType: ObjectChangeType) => {
    notifyWindows(`ObjectChange.${id.className}`, id, changeType);
};