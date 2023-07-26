import m from './module';

export declare abstract class Event {
  readonly topic: number;
}

export declare class UnknownEvent extends Event {}

type EventHandler = (event: Event) => void;
type EventUnsubscribe = () => void;

export declare class EventPipe {
  readonly name: string;

  connect(pipe: EventPipe): void;

  disconnect(pipe: EventPipe): void;

  subscribe(handler: EventHandler): EventUnsubscribe;

  static New(name: string): EventPipe;

  static Combine(pipeIn: EventPipe, pipeOut: EventPipe): EventPipe;
}

module.exports = {
  ...module.exports,
  Event: m.Event,
  EventPipe: m.EventPipe,
};
