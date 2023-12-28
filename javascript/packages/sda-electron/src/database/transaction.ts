import { Database, IDatabaseObject } from './database';

enum ChangeType {
  New,
  Modified,
  Removed,
}

type Change = {
  type: ChangeType;
  obj: IDatabaseObject;
};

export class Transaction {
  private readonly db: Database;
  private changes: { [id: string]: Change } = {};

  constructor(db: Database) {
    this.db = db;
  }

  markAsNew(obj: IDatabaseObject) {
    if (this.changes[obj.id]) {
      throw new Error(`Object ${obj.id} is already marked as new`);
    }
    this.changes[obj.id] = { type: ChangeType.New, obj };
  }

  markAsModified(obj: IDatabaseObject) {
    const change = this.changes[obj.id];
    if (change) {
      if (change.type === ChangeType.Removed) {
        throw new Error(`Object ${obj.id} is marked as removed`);
      }
    } else {
      this.changes[obj.id] = { type: ChangeType.Modified, obj };
    }
  }

  markAsRemoved(obj: IDatabaseObject) {
    const change = this.changes[obj.id];
    if (change) {
      if (change.type === ChangeType.Removed) {
        throw new Error(`Object ${obj.id} is already marked as removed`);
      } else if (change.type === ChangeType.New) {
        delete this.changes[obj.id];
        return;
      }
    }
    this.changes[obj.id] = { type: ChangeType.Removed, obj };
  }

  async commit() {
    for (const [_, change] of Object.entries(this.changes)) {
      const data = change.obj.serialize();
      if (change.type === ChangeType.New || change.type === ChangeType.Modified) {
        await this.db.upsert(data);
      } else if (change.type === ChangeType.Removed) {
        await this.db.delete(data);
      }
    }
    this.changes = {};
  }

  isEmpty() {
    return Object.keys(this.changes).length === 0;
  }
}
