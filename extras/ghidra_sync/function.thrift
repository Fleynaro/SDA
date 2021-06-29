include "shared.thrift"
include "datatype.thrift"

namespace java sda.ghidra.function
namespace cpp ghidra.function

typedef i64 Id

struct SFunctionRange {
	1: i32 minOffset,
	2: i32 maxOffset
}

struct SFunction {
	1: Id id,
	2: string name,
	3: string comment,
	4: datatype.SDataTypeSignature signature,
	5: list<SFunctionRange> ranges
}