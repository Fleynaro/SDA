import m from './module';
import { Event, EventPipe } from './event';
import { Platform } from './platform';
import { SdaObject } from './object';
import { AddressSpace } from './address-space';
import { Hash, IIdentifiable } from './utils';

export declare abstract class ObjectActionEvent extends Event {
  object: SdaObject;
}

export declare class ObjectAddedEvent extends ObjectActionEvent {}

export declare class ObjectModifiedEvent extends ObjectActionEvent {}

export declare class ObjectRemovedEvent extends ObjectActionEvent {}

export declare class ContextRemovedEvent extends Event {
  context: Context;
}

export declare class Context implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly platform: Platform;
  addressSpaces: AddressSpace[];

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
