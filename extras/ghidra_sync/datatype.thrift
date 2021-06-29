include "shared.thrift"

namespace java sda.ghidra.datatype
namespace cpp ghidra.datatype

typedef i64 Id

enum DataTypeGroup
{
	Simple,
	Enum,
	Structure,
	Class,
	Typedef,
	Signature
}

struct SDataType {
	1: Id id,
	2: string name,
	3: string comment,
	4: DataTypeGroup group,
	5: i32 size
}

struct SDataTypeStructureField {
	1: i32 offset,
	2: string name,
	3: string comment,
	4: shared.STypeUnit type
}

struct SDataTypeStructure {
	1: SDataType type,
	2: list<SDataTypeStructureField> fields
}

struct SDataTypeClass {
	1: SDataTypeStructure structType
}

struct SDataTypeEnumField {
	1: string name,
	2: i32 value
}

struct SDataTypeEnum {
	1: SDataType type,
	2: list<SDataTypeEnumField> fields
}

struct SDataTypeTypedef {
	1: SDataType type,
	2: shared.STypeUnit refType
}

struct SFunctionArgument {
	1: string name,
	2: shared.STypeUnit type
}

struct SDataTypeSignature {
	1: SDataType type,
	2: list<SFunctionArgument> arguments,
	3: shared.STypeUnit returnType
}