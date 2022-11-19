import { toId } from '../../utils/common';
import { ContextObject } from 'sda-core/object';
import { ContextObject as ContextObjectDTO } from '../../api/context';

export const toContextObjectDTO = (obj: ContextObject): ContextObjectDTO => {
    return {
        id: toId(obj),
        name: obj.name,
        comment: obj.comment,
    };
};

export const changeContextObject = (obj: ContextObject, dto: ContextObjectDTO): void => {
    obj.name = dto.name;
    obj.comment = dto.comment;
};
