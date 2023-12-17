import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import { DataType, ScalarDataType, ScalarType } from '../data-type';
import { stripSpaces } from './helpers';

describe('DataType', () => {
  let context: Context;

  beforeEach(() => {
    const pipe = EventPipe.New('test');
    const platform = PlatformMock.New();
    context = Context.New(pipe, platform);
  });

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

  it('printer/parser', () => {
    const codeToParse = `
      myDefType = typedef bool
      myDefType2 = typedef myDefType
    `;
    const parsedDataTypes = DataType.Parse(context, codeToParse);
    const codeToPrint = `
      @id '${parsedDataTypes[0].dataType.id}'
      myDefType = typedef bool
      @id '${parsedDataTypes[1].dataType.id}'
      myDefType2 = typedef myDefType
    `;
    const printed = DataType.Print(context, parsedDataTypes);
    expect(stripSpaces(printed)).toBe(stripSpaces(codeToPrint));
  });
});
