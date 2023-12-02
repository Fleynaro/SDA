import m from './module';
import { EventPipe } from './event';
import {
  IRcodeConstant,
  IRcodeFunction,
  IRcodeProgram,
  IRcodeValue,
  IRcodeVariable,
} from './ir-code';

export enum DataFlowNodeType {
  Unknown,
  Start,
  Copy,
  Write,
  Read,
}

export declare class DataFlowNode {
  readonly type: DataFlowNodeType;
  readonly name: string;
  readonly variable: IRcodeVariable;
  readonly constant: IRcodeConstant;
  readonly predecessors: DataFlowNode[];
  readonly successors: DataFlowNode[];
}

export declare class DataFlowRepository {
  readonly globalStartNode: DataFlowNode;

  getNode(value: IRcodeValue): DataFlowNode;

  static New(eventPipe: EventPipe): DataFlowRepository;
}

export declare class DataFlowCollector {
  readonly eventPipe: EventPipe;

  static New(program: IRcodeProgram, dataFlowRepo: DataFlowRepository): DataFlowCollector;
}

export declare function PrintDataFlowForFunction(
  repo: DataFlowRepository,
  func: IRcodeFunction,
): string;

module.exports = {
  ...module.exports,
  DataFlowNode: m.DataFlowNode,
  DataFlowRepository: m.DataFlowRepository,
  DataFlowCollector: m.DataFlowCollector,
  PrintDataFlowForFunction: m.PrintDataFlowForFunction,
};
