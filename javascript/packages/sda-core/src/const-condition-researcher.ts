import m from './module';
import { EventPipe } from './event';
import { IRcodeFunction, IRcodeProgram } from './ir-code';

export declare class ConstConditionRepository {
  readonly eventPipe: EventPipe;

  static New(program: IRcodeProgram): ConstConditionRepository;
}

export declare function PrintConditionsForFunction(
  repo: ConstConditionRepository,
  func: IRcodeFunction,
): string;

module.exports = {
  ...module.exports,
  ConstConditionRepository: m.ConstConditionRepository,
  PrintConditionsForFunction: m.PrintConditionsForFunction,
};
