import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { Image, TestAnalyser, VectorImageRW } from '../image';
import { StandartSymbolTable } from '../symbol-table';

describe('Image', () => {
  const pipe = EventPipe.New('test');
  const platform = PlatformMock.New();
  const context = Context.New(pipe, platform);
  const analyser = TestAnalyser.New();
  const symbolTable = StandartSymbolTable.New(context, '');

  it.skip('VectorImageRW', () => {
    const rw = VectorImageRW.New([0x100, 0x200]);
    const data = rw.read(0, 2);
    expect(data).toEqual([0x100, 0x200]);
  });

  it('New', () => {
    const rw = VectorImageRW.New([0x100]);
    const image = Image.New(context, rw, analyser, 'test', symbolTable);
    expect(image.name).toBe('test');
  });
});
