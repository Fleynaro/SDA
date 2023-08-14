import { toId } from 'utils/common';
import { ContextObject } from 'sda-core';
import { ContextObject as ContextObjectDTO } from 'api/context';

export const toContextObjectDTO = (obj: ContextObject): ContextObjectDTO => {
  return {
    id: toId(obj),
    name: obj.name,
    comment: obj.comment,
    contextId: toId(obj.context),
  };
};

export const changeContextObject = (obj: ContextObject, dto: ContextObjectDTO): void => {
  obj.name = dto.name;
  obj.comment = dto.comment;
};
