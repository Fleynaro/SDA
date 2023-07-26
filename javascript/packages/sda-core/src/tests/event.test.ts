import { EventPipe } from '../event';
import { PlatformMock } from '../platform';
import { Context, ObjectAddedEvent } from '../context';
import { AddressSpace } from '../address-space';
import { instance_of } from 'sda-bindings';

describe('Event', () => {
  describe('EventPipe', () => {
    it('New', () => {
      const pipe = EventPipe.New('test');
      expect(pipe.name).toBe('test');
    });

    it('Combine', () => {
      const pipeIn = EventPipe.New('test1');
      const pipeOut = EventPipe.New('test2');
      const combinedPipe = EventPipe.Combine(pipeIn, pipeOut);
      expect(combinedPipe.name).toBe('Combine');
    });

    it('subscribe', () => {
      const pipe = EventPipe.New('test');
      const platform = PlatformMock.New();
      const context = Context.New(pipe, platform);
      const unsubscribe = pipe.subscribe((event) => {
        if (instance_of(event, ObjectAddedEvent)) {
          const e = event as ObjectAddedEvent;
          if (instance_of(e.object, AddressSpace)) {
            const addressSpace = e.object as AddressSpace;
            expect(addressSpace.name).toBe('my_address_space');
          }
        }
      });
      AddressSpace.New(context, 'my_address_space');
      unsubscribe();
      AddressSpace.New(context, 'my_address_space2');
    });
  });
});
