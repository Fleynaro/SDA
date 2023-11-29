import m from './module';
import { Context } from './context';
import { ContextObject } from './object';
import { Instruction } from './platform';
import { Image, ImageSection } from './image';
import { Offset } from './utils';

export declare function InitLogger(): void;

export declare function CleanUpSharedObjectLookupTable(): void;

export interface InstructionInfo {
  type: Instruction.Type;
  offset: Offset;
  length: number;
  targetOffset: Offset;
  tokens: Instruction.Token[];
}

export declare function GetOriginalInstructions(
  image: Image,
  section: ImageSection,
): InstructionInfo[];

export declare function GetOriginalInstructionInDetail(
  image: Image,
  offset: Offset,
): InstructionInfo;

export declare function CreateContextObject(context: Context, data: object): ContextObject;

module.exports = {
  ...module.exports,
  InitLogger: m.InitLogger,
  CleanUpSharedObjectLookupTable: m.CleanUpSharedObjectLookupTable,
  GetOriginalInstructions: m.GetOriginalInstructions,
  GetOriginalInstructionInDetail: m.GetOriginalInstructionInDetail,
  CreateContextObject: m.CreateContextObject,
};
