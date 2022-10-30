import { Context } from 'sda-core/context';
import { Hash, IWrappable } from 'sda-core/utils';

export class Project implements IWrappable {
    readonly hashId: Hash;
    readonly program: Program;
    readonly path: string;
    readonly context: Context;

    static New(program: Program, path: string, context: Context): Project;

    static Get(hashId: Hash): Project;
}

export class Program implements IWrappable {
    readonly hashId: Hash;
    readonly projects: Project[];

    removeProject(project: Project): void;

    static New(): Program;

    static Get(hashId: Hash): Program;
}

export function CleanUpSharedObjectLookupTable(): void;