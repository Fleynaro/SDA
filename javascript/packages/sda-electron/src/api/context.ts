import { Identifiable } from './common';

export interface ContextObject extends Identifiable {
    name: string;
    comment: string;
}

export interface Context extends Identifiable {
}
