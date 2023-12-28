import m from './module';
import { SdaEvent, EventPipe } from './event';
import { Platform } from './platform';
import { ContextObject } from './object';
import { AddressSpace } from './address-space';
import { Hash, IIdentifiable } from './utils';
import { DataType } from './data-type';

export declare abstract class ObjectActionEvent extends SdaEvent {
  object: ContextObject;
}

export declare class ObjectAddedEvent extends ObjectActionEvent {}

export declare class ObjectModifiedEvent extends ObjectActionEvent {}

export declare class ObjectRemovedEvent extends ObjectActionEvent {}

export declare class ContextRemovedEvent extends SdaEvent {
  context: Context;
}

export declare class Context implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly eventPipe: EventPipe;
  readonly platform: Platform;
  addressSpaces: AddressSpace[];

  getDataType(id: string): DataType;

  static New(pipe: EventPipe, platform: Platform): Context;

  static Get(hashId: Hash): Context;
}

module.exports = {
  ...module.exports,
  ObjectActionEvent: m.ObjectActionEvent,
  ObjectAddedEvent: m.ObjectAddedEvent,
  ObjectModifiedEvent: m.ObjectModifiedEvent,
  ObjectRemovedEvent: m.ObjectRemovedEvent,
  ContextRemovedEvent: m.ContextRemovedEvent,
  Context: m.Context,
};
