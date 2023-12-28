import { Context, DataType } from 'sda-core';
import { AbstractContent, ContentDto } from './abstract-content';

export interface DataTypeDefinition {
  readonly id: string;
  readonly isDeclared: boolean;
}

type DataTypeDefinitionDto = DataTypeDefinition;

export interface DataTypeContentDto extends ContentDto {
  readonly definitions: DataTypeDefinitionDto[];
}

export class DataTypeContent extends AbstractContent {
  private _definitions: DataTypeDefinition[] = [];
  private readonly context: Context;

  private get definitions(): DataTypeDefinition[] {
    return this._definitions;
  }

  private set definitions(definitions: DataTypeDefinition[]) {
    for (const def of this._definitions) {
      this.file.fs.fileIndex.removeFileFromKey(this.file, def.id);
    }
    this._definitions = definitions;
    for (const def of this._definitions) {
      this.file.fs.fileIndex.addFileToKey(this.file, def.id);
    }
  }

  constructor(context: Context) {
    super();
    this.context = context;
  }

  public read(): string {
    const parsedDataTypes = this.definitions.map((def) => {
      const dataType = this.context.getDataType(def.id);
      return {
        ...def,
        dataType,
      };
    });
    return DataType.Print(this.context, parsedDataTypes);
  }

  public write(content: string) {
    const parsedDataTypes = DataType.Parse(this.context, content);
    this.definitions = parsedDataTypes.map(({ dataType, isDeclared }) => ({
      id: dataType.id,
      isDeclared,
    }));
  }

  public deserialize(dto: DataTypeContentDto) {
    this.definitions = dto.definitions.map((def) => ({
      ...def,
    }));
    super.deserialize(dto);
  }

  public serialize(): DataTypeContentDto {
    return {
      ...super.serialize(),
      definitions: this.definitions.map((def) => ({
        ...def,
      })),
    };
  }
}
