import m from './module';
import { Context, Image, ImageSection, Hash, IIdentifiable, Offset, Instruction } from 'sda-core';

export declare class Project implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly program: Program;
  readonly path: string;
  readonly context: Context;
  readonly canBeSaved: boolean;

  load(): void;

  save(): void;

  static New(program: Program, path: string, context: Context): Project;

  static Get(hashId: Hash): Project;
}

export declare abstract class ProgramCallbacks {
  onProjectAdded(project: Project): void;

  onProjectRemoved(project: Project): void;
}

export declare class ProgramCallbacksImpl extends ProgramCallbacks {
  prevCallbacks: ProgramCallbacks;

  onProjectAdded: (project: Project) => void;

  onProjectRemoved: (project: Project) => void;

  static New(): ProgramCallbacksImpl;
}

export declare class Program {
  callbacks: ProgramCallbacks;
  readonly projects: Project[];

  removeProject(project: Project): void;

  static New(): Program;

  static Get(hashId: Hash): Program;
}

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

module.exports = m;
