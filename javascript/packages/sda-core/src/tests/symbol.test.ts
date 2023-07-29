import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { ScalarDataType, ScalarType } from '../data-type';
import { VariableSymbol } from '../symbol';

describe('Symbol', () => {
  let context: Context;
  let dt: ScalarDataType;

  beforeEach(() => {
    const pipe = EventPipe.New('test');
    const platform = PlatformMock.New();
    context = Context.New(pipe, platform);
    dt = ScalarDataType.New(context, 'test', ScalarType.SignedInt, 1);
  });

  it('VariableSymbol', () => {
    const symbol = VariableSymbol.New(context, 'test', dt);
    expect(symbol.name).toBe('test');
  });
});
