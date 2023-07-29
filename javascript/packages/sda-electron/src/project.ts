import { instance_of } from 'sda-bindings';
import {
  Context,
  CreateContextObject,
  EventPipe,
  Hash,
  IIdentifiable,
  ObjectAddedEvent,
  ObjectModifiedEvent,
  ObjectRemovedEvent,
} from 'sda-core';
import { Database, Transaction } from 'database';

let ProjectId = 0;

export class Project implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly path: string;
  readonly context: Context;
  private readonly eventPipe: EventPipe;
  private readonly database: Database;
  private readonly transaction: Transaction;

  constructor(path: string, context: Context) {
    this.path = path;
    this.context = context;
    this.className = 'Project';
    this.hashId = ProjectId++;
    this.eventPipe = EventPipe.New('project');
    this.database = new Database(path + '/database.sqlite');
    this.transaction = new Transaction(this.database);
  }

  async init() {
    this.initEventPipe();
    await this.database.connect();
    this.initDatabase();
  }

  initEventPipe() {
    this.context.eventPipe.connect(this.eventPipe);
    this.eventPipe.subscribe((event) => {
      if (instance_of(event, ObjectAddedEvent)) {
        const e = event as ObjectAddedEvent;
        this.transaction.markAsNew(e.object);
      } else if (instance_of(event, ObjectModifiedEvent)) {
        const e = event as ObjectModifiedEvent;
        this.transaction.markAsModified(e.object);
      } else if (instance_of(event, ObjectRemovedEvent)) {
        const e = event as ObjectRemovedEvent;
        this.transaction.markAsRemoved(e.object);
      }
    });
  }

  private initDatabase() {
    this.database.addTable('address_space');
    this.database.addTable('image');
    this.database.addTable('data_type');
    this.database.addTable('symbol');
    this.database.addTable('symbol_table');
  }

  async load() {
    this.context.eventPipe.disconnect(this.eventPipe);
    const objects = (await this.database.loadAll()).map((data) => ({
      data,
      ctxObj: CreateContextObject(this.context, data),
    }));
    for (const { data, ctxObj } of objects) {
      ctxObj.deserialize(data);
    }
    this.context.eventPipe.connect(this.eventPipe);
  }

  async save() {
    await this.transaction.commit();
  }

  canBeSaved() {
    return !this.transaction.isEmpty();
  }

  destroy() {
    this.context.eventPipe.disconnect(this.eventPipe);
  }
}
