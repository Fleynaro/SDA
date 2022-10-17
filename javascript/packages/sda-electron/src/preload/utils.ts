import { ipcRenderer } from 'electron';

export const invokerFactory = (controllerName: string) => {
    return (methodName: string, ...args: any[]) => {
        return ipcRenderer.invoke(controllerName + '.' + methodName, ...args);
    }
}