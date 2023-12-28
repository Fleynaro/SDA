import { Context, CreateContextObject, EventPipe, Hash, IIdentifiable } from 'sda-core';
import { Database, Transaction } from 'database';
import { CreateFileSystemDataBasePipe, FILESYSTEM_DATABASE_TABLE, FileSystem } from 'file-system';
import { CreateSdaDataBasePipe, SDA_DATABASE_TABLES } from 'sda-database';
import { FileDto } from 'file-system/file';
import { CreateGuiEventEmitterPipe } from 'file-system/event-emitter';

let ProjectId = 0;

export class Project implements IIdentifiable {
  readonly hashId: Hash;
  readonly className: string;
  readonly path: string;
  readonly context: Context;
  private readonly eventPipe: EventPipe;
  private readonly database: Database;
  private readonly transaction: Transaction;
  public readonly fileSystem: FileSystem;

  constructor(path: string, context: Context) {
    this.path = path;
    this.context = context;
    this.className = 'Project';
    this.hashId = (ProjectId++).toString();
    this.eventPipe = EventPipe.New('project');
    this.database = new Database(path + '/database.sqlite');
    this.transaction = new Transaction(this.database);
    this.fileSystem = new FileSystem('main', this, this.eventPipe);
  }

  async init() {
    this.initEventPipe();
    await this.database.connect();
    this.initDatabase();
  }

  initEventPipe() {
    this.context.eventPipe.connect(this.eventPipe);
    this.eventPipe.connect(CreateSdaDataBasePipe(this.transaction));
    this.eventPipe.connect(CreateFileSystemDataBasePipe(this.transaction));
    this.eventPipe.connect(CreateGuiEventEmitterPipe());
  }

  private initDatabase() {
    SDA_DATABASE_TABLES.map((table) => this.database.addTable(table));
    this.database.addTable(FILESYSTEM_DATABASE_TABLE);
  }

  async load() {
    await this.loadSdaObjects();
    await this.loadFileSystem();
  }

  private async loadSdaObjects() {
    this.context.eventPipe.disconnect(this.eventPipe);
    const objects = (await this.database.loadAll(SDA_DATABASE_TABLES)).map((dto) => ({
      dto,
      ctxObj: CreateContextObject(this.context, dto),
    }));
    for (const { dto, ctxObj } of objects) {
      ctxObj.deserialize(dto);
    }
    this.context.eventPipe.connect(this.eventPipe);
  }

  private async loadFileSystem() {
    const prevPipe = this.fileSystem.pipe;
    this.fileSystem.pipe = EventPipe.New('temp');
    const files = (await this.database.loadAll([FILESYSTEM_DATABASE_TABLE])).map((dto) => ({
      dto: dto as FileDto,
      file: this.fileSystem.createFileObject(dto as FileDto),
    }));
    for (const { dto, file } of files) {
      file.deserialize(dto);
    }
    this.fileSystem.pipe = prevPipe;
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
