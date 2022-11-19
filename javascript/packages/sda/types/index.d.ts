import { Context } from 'sda-core/context';
import { Hash, IIdentifiable } from 'sda-core/utils';

export class Project implements IIdentifiable {
    readonly hashId: Hash;
    readonly className: string;
    readonly program: Program;
    readonly path: string;
    readonly context: Context;

    static New(program: Program, path: string, context: Context): Project;

    static Get(hashId: Hash): Project;
}

export abstract class ProgramCallbacks {
    onProjectAdded(project: Project): void;

    onProjectRemoved(project: Project): void;
}

export class ProgramCallbacksImpl extends ProgramCallbacks {
    oldCallbacks: ProgramCallbacks;

    onProjectAdded: (project: Project) => void;

    onProjectRemoved: (project: Project) => void;

    static New(): ProgramCallbacksImpl;
}

export class Program {
    callbacks: ProgramCallbacks;
    readonly projects: Project[];

    removeProject(project: Project): void;

    static New(): Program;

    static Get(hashId: Hash): Program;
}

export function CleanUpSharedObjectLookupTable(): void;