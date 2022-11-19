import { app } from "electron";
import path from "path";
import { existsSync, mkdirSync } from "fs";
import { initControllers } from "./controllers";
import { Program, ProgramCallbacksImpl, CleanUpSharedObjectLookupTable } from 'sda';
import { initDefaultPlatforms } from './sda/platform';
import { initDefaultImageAnalysers } from './sda/image-analyser';
import { objectChangeEmitter, ObjectChangeType } from './eventEmitter';
import { toId } from './utils/common';

export let program: Program;

export const getUserDir = () => {
    return path.join(app.getPath("documents"), "SDA");
}

export const getUserPath = (file: string) => {
    return path.join(getUserDir(), file);
}

export const initApp = () => {
    program = Program.New();
    {
        const callbacks = ProgramCallbacksImpl.New();
        callbacks.oldCallbacks = program.callbacks;
        callbacks.onProjectAdded = (project) =>
            objectChangeEmitter()(toId(project), ObjectChangeType.Create);
        callbacks.onProjectRemoved = (project) =>
            objectChangeEmitter()(toId(project), ObjectChangeType.Delete);
        program.callbacks = callbacks;
    }
    initDefaultPlatforms();
    initDefaultImageAnalysers();
    initControllers();

    // create user directory if not exists
    const userDir = getUserDir();
    if (!existsSync(userDir)) {
        mkdirSync(userDir);
    }

    setInterval(() => { // TODO: remove when app exit
        CleanUpSharedObjectLookupTable();
    }, 10000);
}