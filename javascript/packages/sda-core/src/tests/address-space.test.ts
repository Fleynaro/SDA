import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { AddressSpace } from '../address-space';
import { Image, TestAnalyser, VectorImageRW } from '../image';
import { StandartSymbolTable } from '../symbol-table';

describe('AddressSpace', () => {
  let context: Context;

  beforeEach(() => {
    const pipe = EventPipe.New('test');
    const platform = PlatformMock.New();
    context = Context.New(pipe, platform);
  });

  it('New', () => {
    const addrSpace = AddressSpace.New(context, 'test');
    expect(addrSpace.name).toBe('test');
  });

  describe('methods', () => {
    let addrSpace: AddressSpace;

    beforeEach(() => {
      addrSpace = AddressSpace.New(context, 'test');
    });

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
