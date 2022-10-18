import { Context } from 'sda-core/context';
import { Context as ContextDTO } from '../api/context';
import { ObjectId } from '../api/common';

export const toContextId = (project: Context): ObjectId => {
    return {
        key: project.hashId.toString(),
        className: 'Context',
    };
};

export const toContextDTO = (context: Context): ContextDTO => {
    return {
        id: toContextId(context),
    };
};