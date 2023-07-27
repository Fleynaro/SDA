import { EventPipe, JsEvent, IEvent } from '../event';
import { instance_of } from 'sda-bindings';
import { StringToHash } from '../utils';

class TestEvent extends JsEvent {
  readonly value: number;
  constructor(value: number) {
    super('Test');
    this.value = value;
  }

  static OnlyEven(event: IEvent) {
    if (instance_of(event, TestEvent)) {
      const e = event as unknown as TestEvent;
      return e.value % 2 === 0; // only even values
    }
    return false;
  }
}

describe('Event', () => {
  describe('EventPipe', () => {
    it('New', () => {
      const pipe = EventPipe.New('test');
      expect(pipe.name).toBe('test');
    });

    it('send/subscribe', () => {
      const pipe = EventPipe.New('test');
      const unsubscribe = pipe.subscribe((event) => {
        if (instance_of(event, TestEvent)) {
          const e = event as TestEvent;
          expect(e.topic).toBe(StringToHash('Test'));
          expect(e.value).toBe(100);
        }
      });
      pipe.send(new TestEvent(100));
      unsubscribe();
      pipe.send(new TestEvent(101));
    });

    it('process', () => {
      const pipe = EventPipe.New('test');
      pipe
        .process((event, next) => {
          if (instance_of(event, TestEvent)) {
            const e = event as TestEvent;
            next(new TestEvent(e.value + 10));
          }
        })
        .subscribe((event) => {
          if (instance_of(event, TestEvent)) {
            const e = event as TestEvent;
            expect(e.value).toBe(110);
          }
        });
      pipe.send(new TestEvent(100));
    });

    it('filter', () => {
      const pipe = EventPipe.New('test');
      pipe.filter(TestEvent.OnlyEven).subscribe((event) => {
        if (instance_of(event, TestEvent)) {
          const e = event as TestEvent;
          expect(e.value).toBe(12);
        }
      });
      pipe.send(new TestEvent(11));
      pipe.send(new TestEvent(12));
    });

    it('Combine', () => {
      const pipeIn = EventPipe.New('test1');
      const pipeOut = EventPipe.New('test2');
      const combinedPipe = EventPipe.Combine(pipeIn, pipeOut);
      expect(combinedPipe.name).toBe('Combine');
    });

    it('If', () => {
      const pipeThen = EventPipe.New('test1');
      const pipeElse = EventPipe.New('test2');
      const combinedPipe = EventPipe.If(TestEvent.OnlyEven, pipeThen, pipeElse);
      expect(combinedPipe.name).toBe('If');
    });
  });
});
