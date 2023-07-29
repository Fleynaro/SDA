import { PlatformMock } from '../platform';
import { Context } from '../context';
import { EventPipe } from '../event';
import {
  PcodeGraph,
  PcodeInstructionId,
  PcodeParser,
  PcodePrinter,
  toInstructionOffset,
} from '../p-code';

describe('P-code', () => {
  let context: Context;

  beforeEach(() => {
    const pipe = EventPipe.New('test');
    const platform = PlatformMock.New();
    context = Context.New(pipe, platform);
  });

  it('parser/printer', () => {
    const source = `
      $1:8 = COPY 0:8
      CBRANCH <labelElse>, 0:1 // if-else condition
      $1:8 = COPY 1:8 // then block
      BRANCH <labelEnd>
      <labelElse>:
      NOP // else block
      <labelEnd>:
      $2:8 = INT_2COMP $1:8
    `;
    const expected = `
      $U1:8 = COPY 0x0:8
      CBRANCH 0x400:8, 0x0:1
      $U1:8 = COPY 0x1:8
      BRANCH 0x500:8
      NOP
      $U2:8 = INT_2COMP $U1:8
    `;
    const instructions = PcodeParser.Parse(source, null);
    const printed = instructions.map((instr) => PcodePrinter.Print(instr, null)).join('\n');
    expect(printed.replace(/\s/g, '')).toBe(expected.replace(/\s/g, ''));
  });

  it('explore', () => {
    const source = `
      $1:8 = COPY 0:8
      CBRANCH <labelElse>, 0:1 // if-else condition
      $1:8 = COPY 1:8 // then block
      BRANCH <labelEnd>
      <labelElse>:
      NOP // else block
      <labelEnd>:
      $2:8 = INT_2COMP $1:8
    `;
    const instructions = PcodeParser.Parse(source, null);
    const pcodeGraph = PcodeGraph.New(context.eventPipe, context.platform);
    pcodeGraph.exploreInstructions(0, instructions);
    expect(pcodeGraph.getFunctionGraphAt(toInstructionOffset(0x0)).name).toBe('B0');
    expect(pcodeGraph.getBlockAt(toInstructionOffset(0x5), false).name).toBe('B5');
    expect(pcodeGraph.getInstructionAt(toInstructionOffset(0x5)).id).toBe(
      PcodeInstructionId.INT_2COMP,
    );
  });
});
