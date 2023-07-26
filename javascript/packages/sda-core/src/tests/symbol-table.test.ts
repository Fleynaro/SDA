import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { StandartSymbolTable } from '../symbol-table';
import { VariableSymbol } from '../symbol';
import { ScalarDataType, ScalarType } from '../data-type';

describe('SymbolTable', () => {
  const pipe = EventPipe.New('test');
  const platform = PlatformMock.New();
  const context = Context.New(pipe, platform);
  const dt = ScalarDataType.New(context, 'test', ScalarType.SignedInt, 4);

  describe('Standart', () => {
    it('New', () => {
      const symbolTable = StandartSymbolTable.New(context, 'test');
      expect(symbolTable.name).toBe('test');
    });

    describe('methods', () => {
      const symbolTable = StandartSymbolTable.New(context, 'test');

      it('addSymbol', () => {
        symbolTable.addSymbol(0, VariableSymbol.New(context, 'var1', dt));
        expect(symbolTable.getAllSymbols().length).toBe(1);
      });
    });
  });
});
