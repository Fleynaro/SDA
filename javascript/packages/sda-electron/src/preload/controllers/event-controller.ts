import { subscribeToEvent } from '../utils';
import { EventController, EventName } from 'api/event';

const EventControllerImpl: EventController = {
  subscribeToObjectChangeEvent: (callback) => subscribeToEvent(EventName.ObjectChange, callback),
};

export default EventControllerImpl;
