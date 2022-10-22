import { ipcRenderer } from 'electron';
import { EventController, EventName } from '../../api/event';

const subscribeToEvent = (event: EventName, callback: (...args: any[]) => void): () => void => {
    const channel = `Event.${event}`;
    const subscription = (event, ...args: any[]) => callback(...args);
    ipcRenderer.on(channel, subscription);
    return () => ipcRenderer.removeListener(channel, subscription);
};

const EventControllerImpl: EventController = {
    subscribeToWindowOpenEvent: (callback) =>
        subscribeToEvent(EventName.WindowOpen, callback),

    subscribeToObjectChangeEvent: (callback) =>
        subscribeToEvent(EventName.ObjectChange, callback)
};

export default EventControllerImpl;