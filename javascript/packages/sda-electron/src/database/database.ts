import sqlite3 from 'sqlite3';
import sqlite, { open } from 'sqlite';
import assert from 'assert';

export interface DatabaseObject {
  class: string;
  uuid: string;
}

export class Database {
  private readonly path: string;
  private db?: sqlite.Database<sqlite3.Database, sqlite3.Statement>;
  private tables: string[] = [];

  constructor(path: string) {
    this.path = path;
  }

  async connect() {
    this.db = await open({
      filename: this.path,
      driver: sqlite3.Database,
    });
  }

  addTable(name: string) {
    this.tables.push(name);
    this.createTableIfNotExists(name);
  }

  private async createTableIfNotExists(name: string) {
    assert(this.db);
    return this.db.run(
      `CREATE TABLE IF NOT EXISTS ${name} (uuid TEXT PRIMARY KEY NOT NULL, data TEXT NOT NULL)`,
    );
  }

  async loadAll() {
    assert(this.db);
    const objects: DatabaseObject[] = [];
    for (const table of this.tables) {
      const objectsInTable = await this.db.all(`SELECT * FROM ${table}`);
      for (const obj of objectsInTable) {
        const { data } = obj as { data: string };
        objects.push(JSON.parse(data));
      }
    }
    return objects;
  }

  async upsert(obj: DatabaseObject) {
    assert(this.db);
    const data = JSON.stringify(obj);
    return this.db.run(`REPLACE INTO ${obj.class} (uuid, data) VALUES (?, ?)`, obj.uuid, data);
  }

  async delete(obj: DatabaseObject) {
    assert(this.db);
    return this.db.run(`DELETE FROM ${obj.class} WHERE uuid = ?`, obj.uuid);
  }
}
