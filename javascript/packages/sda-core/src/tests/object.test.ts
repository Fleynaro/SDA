import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { ScalarDataType, ScalarType } from '../data-type';
import { CreateContextObject } from '../helpers';

describe('Object', () => {
  const pipe = EventPipe.New('test');
  const platform = PlatformMock.New();
  const context = Context.New(pipe, platform);

  it('serialize/deserialize', () => {
    const dt1 = ScalarDataType.New(context, 'test', ScalarType.SignedInt, 1);
    const data1 = dt1.serialize();

    const context2 = Context.New(pipe, platform);
    const dt2 = CreateContextObject(context2, data1);
    // after creation, it also should be deserialized
    dt2.deserialize(data1);
    const data2 = dt2.serialize();

    expect(data1).toEqual(data2);
  });
});
