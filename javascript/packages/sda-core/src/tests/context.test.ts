import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';

describe('Context', () => {
  const pipe = EventPipe.New('test');
  const platform = PlatformMock.New();
  const context = Context.New(pipe, platform);

  it('className', () => {
    expect(context.className).toBe('Context');
  });
});
