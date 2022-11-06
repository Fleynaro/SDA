import { Context } from 'sda-core/context';
import { ContextObject } from 'sda-core/object';
import { Context as ContextDTO, ContextObject as ContextObjectDTO } from '../api/context';
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

export const toContextObjectDTO = (obj: ContextObject): Omit<ContextObjectDTO, 'id'> => {
    return {
        name: obj.name,
        comment: obj.comment,
    };
};