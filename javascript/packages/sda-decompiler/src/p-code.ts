import m from './module';
import { PcodeGraph, Image, PcodeDecoder, InstructionOffset } from 'sda-core';

export declare class PcodeGraphBuilder {
  start(startOffsets: InstructionOffset[], fromEntryPoints: boolean): void;

  static New(graph: PcodeGraph, image: Image, decoder: PcodeDecoder): PcodeGraphBuilder;
}

module.exports = {
  ...module.exports,
  PcodeGraphBuilder: m.PcodeGraphBuilder,
};
