import m from './module';
import { EventPipe } from './event';
import { IRcodeProgram, IRcodeVariable } from './ir-code';
import { DataFlowRepository } from './data-flow-researcher';
import { ClassRepository } from './class-researcher';
import { Hash } from './utils';

export declare abstract class Semantics {
  readonly name: string;
  readonly hash: Hash;
  readonly successors: Semantics[];
  readonly predecessors: Semantics[];
}

export declare class HolderSemantics extends Semantics {
  readonly holder: SemanticsObject;
  readonly semantics: Semantics;
}

export declare class SemanticsObject {
  readonly semantics: HolderSemantics[];
  readonly variables: IRcodeVariable[];
}

export declare abstract class SemanticsPropagator {}

export declare class BaseSemanticsPropagator extends SemanticsPropagator {
  static New(
    program: IRcodeProgram,
    semanticsRepo: SemanticsRepository,
    dataFlowRepo: DataFlowRepository,
  ): BaseSemanticsPropagator;
}

export declare class SemanticsRepository {
  readonly allObjects: SemanticsObject[];

  getObject(variable: IRcodeVariable): SemanticsObject;

  getSemantics(hash: Hash): Semantics;

  static New(eventPipe: EventPipe): SemanticsRepository;
}

export declare class SemanticsResearcher {
  readonly eventPipe: EventPipe;

  addPropagator(propagator: SemanticsPropagator): void;

  static New(
    program: IRcodeProgram,
    semanticsRepo: SemanticsRepository,
    classCondRepo: ClassRepository,
    dataFlowRepo: DataFlowRepository,
  ): SemanticsResearcher;
}

module.exports = {
  ...module.exports,
  Semantics: m.Semantics,
  HolderSemantics: m.HolderSemantics,
  SemanticsObject: m.SemanticsObject,
  SemanticsPropagator: m.SemanticsPropagator,
  BaseSemanticsPropagator: m.BaseSemanticsPropagator,
  SemanticsRepository: m.SemanticsRepository,
  SemanticsResearcher: m.SemanticsResearcher,
};
