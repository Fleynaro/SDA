import m from './module';
import { Context } from './context';
import { IIdentifiable, ISerializable, Hash } from './utils';

export declare abstract class SdaObject implements IIdentifiable, ISerializable {
  readonly hashId: Hash;
  readonly className: string;
  readonly id: string;

  setTemporary(temporary: boolean): void;

  serialize(): object;

  deserialize(data: object): void;

  static Get(hashId: Hash): SdaObject;
}

export declare abstract class ContextObject extends SdaObject {
  name: string;
  comment: string;
  context: Context;
}

module.exports = {
  ...module.exports,
  SdaObject: m.SdaObject,
  ContextObject: m.ContextObject,
};
