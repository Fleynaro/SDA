import { app } from "electron";
import path from "path";
import { existsSync, mkdirSync } from "fs";
import { initControllers } from "./controllers";
import { Program } from 'sda';
import { initDefaultPlatforms } from './sda/platform';

export let program: Program;

export const getUserDir = () => {
    return path.join(app.getPath("documents"), "SDA");
}

export const getUserPath = (file: string) => {
    return path.join(getUserDir(), file);
}

export const initApp = () => {
    program = Program.New();
    initDefaultPlatforms();
    initControllers();

    // create user directory if not exists
    const userDir = getUserDir();
    if (!existsSync(userDir)) {
        mkdirSync(userDir);
    }
}