import sqlite3 from 'sqlite3';

export interface DatabaseObject {
  class: string;
  uuid: string;
}

export class Database {
  private readonly path: string;
  private readonly db: sqlite3.Database;
  private tables: string[] = [];

  constructor(path: string) {
    this.path = path;
    this.db = new sqlite3.Database(path);
  }

  addTable(name: string) {
    this.tables.push(name);
    this.createTableIfNotExists(name);
  }

  private createTableIfNotExists(name: string) {
    this.db.serialize(() => {
      this.db.run(
        `CREATE TABLE IF NOT EXISTS ${name} (uuid TEXT PRIMARY KEY NOT NULL, data TEXT NOT NULL)`,
      );
    });
  }

  loadAll() {
    const objects: DatabaseObject[] = [];
    this.db.serialize(() => {
      for (const table of this.tables) {
        this.db.each(`SELECT * FROM ${table}`, (err, row) => {
          if (err) {
            throw err;
          }
          const { data } = row as { data: string };
          objects.push(JSON.parse(data));
        });
      }
    });
    return objects;
  }

  upsert(obj: DatabaseObject) {
    const data = JSON.stringify(obj);
    this.db.serialize(() => {
      this.db.run(`REPLACE INTO ${obj.class} (uuid, data) VALUES (?, ?)`, [obj.uuid, data]);
    });
  }

  delete(obj: DatabaseObject) {
    this.db.serialize(() => {
      this.db.run(`DELETE FROM ${obj.class} WHERE uuid = ?`, [obj.uuid]);
    });
  }
}
