import { EventName, WindowOpenEventCallback, ObjectChangeEventCallback } from './api/event';
import { BrowserWindow, sendMessageToWindow, sendMessageToAllWindows } from "./utils/window";

export { ObjectChangeType } from './api/common';

const event = (name: EventName) => `Event.${name}`

export const windowOpenEmitter = (window: BrowserWindow) =>
    function(name, payload) {
        sendMessageToWindow(window, event(EventName.WindowOpen), name, payload);
    } as WindowOpenEventCallback;

export const objectChangeEmitter = () =>
    function(id, changeType) {
        sendMessageToAllWindows(event(EventName.ObjectChange), id, changeType);
    } as ObjectChangeEventCallback;