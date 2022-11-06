import { Hash } from 'sda-core/utils';
import { ObjectId } from '../api/common';

export const toHash = (id: ObjectId): Hash => {
    return Number(id.key);
};