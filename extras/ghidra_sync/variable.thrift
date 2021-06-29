include "shared.thrift"

namespace java sda.ghidra.variable
namespace cpp ghidra.variable

typedef i64 Id

struct SGlobalVar {
	1: Id id,
	2: string name,
	3: string comment,
	4: shared.STypeUnit type
}