import { PlatformMock } from '../platform';

describe('Platform', () => {
  const platform = PlatformMock.New();

  it('className', () => {
    expect(platform.className).toBe('Platform');
  });
});
