#include "StructFieldSymbol.h"
#include <datatypes/Structure.h>

CE::Symbol::Type CE::Symbol::StructFieldSymbol::getType() {
	return STRUCT_FIELD;
}

int CE::Symbol::StructFieldSymbol::getBitSize() const
{
	return m_bitSize;
}

int& CE::Symbol::StructFieldSymbol::getAbsBitOffset() {
	return m_absBitOffset;
}

int CE::Symbol::StructFieldSymbol::getBitOffset() {
	return m_absBitOffset - getOffset() * 0x8;
}

int CE::Symbol::StructFieldSymbol::getOffset() {
	auto byteOffset = m_absBitOffset / 0x8;
	return byteOffset - (isBitField() ? (byteOffset % getSize()) : 0);
}

bool CE::Symbol::StructFieldSymbol::isBitField() const
{
	return (m_bitSize % 0x8) != 0 || (m_absBitOffset % 0x8) != 0;
}

void CE::Symbol::StructFieldSymbol::setStructure(DataType::IStructure* structure) {
	m_structure = structure;
}

bool CE::Symbol::StructFieldSymbol::isDefault() const
{
	return m_isDefault;
}
