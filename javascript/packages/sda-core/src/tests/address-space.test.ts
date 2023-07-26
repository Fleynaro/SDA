import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { AddressSpace } from '../address-space';
import { Image, TestAnalyser, VectorImageRW } from '../image';
import { StandartSymbolTable } from '../symbol-table';

describe('AddressSpace', () => {
  const pipe = EventPipe.New('test');
  const platform = PlatformMock.New();
  const context = Context.New(pipe, platform);

  it('New', () => {
    const addrSpace = AddressSpace.New(context, 'test');
    expect(addrSpace.name).toBe('test');
  });

  describe('methods', () => {
    const addrSpace = AddressSpace.New(context, 'test');

    it('images', () => {
      const analyser = TestAnalyser.New();
      const rw = VectorImageRW.New([]);
      const symbolTable = StandartSymbolTable.New(context, '');
      addrSpace.images = [Image.New(context, rw, analyser, 'test', symbolTable)];
      expect(addrSpace.images.length).toBe(1);
      expect(addrSpace.images[0].name).toBe('test');
    });
  });
});
