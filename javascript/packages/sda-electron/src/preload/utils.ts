import { ipcRenderer } from 'electron';

export const invokerFactory = (controllerName: string) => {
  return (methodName: string, ...args: any[]) => {
    return ipcRenderer.invoke(controllerName + '.' + methodName, ...args);
  };
};

export const subscribeToEvent = (
  channel: string,
  callback: (...args: any[]) => void,
): (() => void) => {
  const subscription = (event, ...args: any[]) => callback(...args);
  ipcRenderer.on(channel, subscription);
  return () => ipcRenderer.removeListener(channel, subscription);
};
