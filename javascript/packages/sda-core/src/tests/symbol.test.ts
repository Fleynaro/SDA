import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { ScalarDataType, ScalarType } from '../data-type';
import { VariableSymbol } from '../symbol';

describe('Symbol', () => {
  const pipe = EventPipe.New('test');
  const platform = PlatformMock.New();
  const context = Context.New(pipe, platform);
  const dt = ScalarDataType.New(context, 'test', ScalarType.SignedInt, 1);

  it('VariableSymbol', () => {
    const symbol = VariableSymbol.New(context, 'test', dt);
    expect(symbol.name).toBe('test');
  });
});
