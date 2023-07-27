import m from './module';
import { StringToHash } from './utils';

export interface IEvent {
  readonly topic: number;
}

export declare abstract class SdaEvent implements IEvent {
  readonly topic: number;
}

export abstract class JsEvent implements IEvent {
  readonly topic: number;

  constructor(topic: string) {
    this.topic = StringToHash(topic);
  }
}

export declare class UnknownEvent extends SdaEvent {}

type EventNext = (event: IEvent) => void;
type EventProcessor = (event: IEvent, next: EventNext) => void;
type EventFilter = (event: IEvent) => boolean;
type EventHandler = (event: IEvent) => void;
type EventUnsubscribe = () => void;

export declare class EventPipe {
  readonly name: string;

  connect(pipe: EventPipe): void;

  disconnect(pipe: EventPipe): void;

  process(processor: EventProcessor): EventPipe;

  filter(filter: EventFilter): EventPipe;

  send(event: JsEvent): void;

  subscribe(handler: EventHandler): EventUnsubscribe;

  static New(name: string): EventPipe;

  static Combine(pipeIn: EventPipe, pipeOut: EventPipe): EventPipe;

  static If(condition: EventFilter, pipeThen: EventPipe, pipeElse: EventPipe): EventPipe;
}

module.exports = {
  ...module.exports,
  SdaEvent: m.SdaEvent,
  EventPipe: m.EventPipe,
};
