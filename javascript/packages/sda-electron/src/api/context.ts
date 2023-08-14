import { Identifiable, ObjectId } from './common';

export interface ContextObject extends Identifiable {
  name: string;
  comment: string;
  contextId: ObjectId;
}
