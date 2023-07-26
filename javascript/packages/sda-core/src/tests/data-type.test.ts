import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { ScalarDataType, ScalarType } from '../data-type';

describe('DataType', () => {
  const pipe = EventPipe.New('test');
  const platform = PlatformMock.New();
  const context = Context.New(pipe, platform);

  it('ScalarDataType', () => {
    const dt = ScalarDataType.New(context, 'test', ScalarType.SignedInt, 1);
    expect(dt.name).toBe('test');
    expect(dt.isSigned).toBe(true);
    expect([dt.isFloatingPoint, dt.isUnsigned, dt.isPointer, dt.isVoid]).toEqual([
      false,
      false,
      false,
      false,
    ]);
  });
});
