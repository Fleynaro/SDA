import { Hash, IIdentifiable } from 'sda-core/utils';
import { ObjectId } from '../api/common';

export const toHash = (id: ObjectId): Hash => {
    return Number(id.key);
};

export const toId = (object: IIdentifiable): ObjectId => {
    return {
        key: object.hashId.toString(),
        className: object.className,
    };
};