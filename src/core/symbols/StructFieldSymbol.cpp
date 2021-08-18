#include "StructFieldSymbol.h"

CE::Symbol::Type CE::Symbol::StructFieldSymbol::getType() {
	return STRUCT_FIELD;
}

CE::Decompiler::Storage CE::Symbol::StructFieldSymbol::getStorage() {
	return Decompiler::Storage();
}

int CE::Symbol::StructFieldSymbol::getBitSize() const {
	return m_bitSize;
}

void CE::Symbol::StructFieldSymbol::setBitSize(int size) {
	m_bitSize = size;
}

int CE::Symbol::StructFieldSymbol::getAbsBitOffset() const {
	return m_absBitOffset;
}

void CE::Symbol::StructFieldSymbol::setAbsBitOffset(int offset) {
	m_absBitOffset = offset;
}

int CE::Symbol::StructFieldSymbol::getBitOffset() {
	return m_absBitOffset - getOffset() * 0x8;
}

int CE::Symbol::StructFieldSymbol::getOffset() {
	const auto byteOffset = m_absBitOffset / 0x8;
	return byteOffset - (isBitField() ? byteOffset % getSize() : 0);
}

bool CE::Symbol::StructFieldSymbol::isBitField() {
	return m_bitSize != getSize() * 0x8;
}

bool CE::Symbol::StructFieldSymbol::isDefault() const
{
	return m_isDefault;
}
