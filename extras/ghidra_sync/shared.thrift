namespace java sda.ghidra.shared
namespace cpp ghidra.shared

typedef i64 Id

struct STypeUnit {
	1: Id typeId,
	2: list<i16> pointerLvls
}