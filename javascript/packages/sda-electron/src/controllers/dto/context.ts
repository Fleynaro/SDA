import { Context } from 'sda-core/context';
import { ObjectId } from 'api/common';
import { toHash } from 'utils/common';

export const toContextId = (project: Context): ObjectId => {
  return {
    key: project.hashId.toString(),
    className: 'Context',
  };
};

export const toContext = (id: ObjectId): Context => {
  const context = Context.Get(toHash(id));
  if (!context) {
    throw new Error(`Context ${id.key} does not exist`);
  }
  return context;
};
