import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { StandartSymbolTable } from '../symbol-table';
import { VariableSymbol } from '../symbol';
import { ScalarDataType, ScalarType } from '../data-type';

describe('SymbolTable', () => {
  let context: Context;
  let dt: ScalarDataType;

  beforeEach(() => {
    const pipe = EventPipe.New('test');
    const platform = PlatformMock.New();
    context = Context.New(pipe, platform);
    dt = ScalarDataType.New(context, 'test', ScalarType.SignedInt, 1);
  });

  describe('Standart', () => {
    it('New', () => {
      const symbolTable = StandartSymbolTable.New(context, 'test');
      expect(symbolTable.name).toBe('test');
    });

    describe('methods', () => {
      let symbolTable: StandartSymbolTable;
      beforeEach(() => {
        symbolTable = StandartSymbolTable.New(context, 'test');
      });

      it('addSymbol', () => {
        symbolTable.addSymbol(0, VariableSymbol.New(context, 'var1', dt));
        expect(symbolTable.getAllSymbols().length).toBe(1);
      });
    });
  });
});
