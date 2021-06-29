#include "Structure.h"
#include <Manager/TypeManager.h>
#include <Manager/SymbolManager.h>

using namespace CE;
using namespace CE::DataType;

CE::DataType::Structure::Structure(TypeManager* typeManager, const std::string& name, const std::string& comment)
	: UserDefinedType(typeManager, name, comment)
{
	auto factory = typeManager->getProject()->getSymbolManager()->getFactory(false);
	m_defaultField = factory.createStructFieldSymbol(-1, -1, this, GetUnit(typeManager->getFactory().getDefaultType()), "undefined");
}

Structure::~Structure() {
	for (auto it : m_fields)
		delete it.second;
	delete m_defaultField;
}

AbstractType::Group Structure::getGroup() {
	return Group::Structure;
}

int Structure::getSize() {
	return m_size;
}

void Structure::resize(int size) {
	m_size = size;
}

int Structure::getSizeByLastField() {
	if (m_fields.empty())
		return 0;
	auto lastField = std::prev(m_fields.end())->second;
	return lastField->getOffset() + lastField->getSize();
}

Structure::FieldMapType& Structure::getFields() {
	return m_fields;
}

int Structure::getNextEmptyBitsCount(int bitOffset) {
	auto it = m_fields.upper_bound(bitOffset);
	if (it != m_fields.end()) {
		return it->first - bitOffset;
	}
	return m_size - bitOffset;
}

bool Structure::areEmptyFields(int bitOffset, int bitSize) {
	if (bitOffset < 0 || bitSize <= 0)
		return false;

	//check free space to the next field starting at the bitOffset
	if (getNextEmptyBitsCount(bitOffset) < bitSize)
		return false;

	//check intersecting with an existing field at the bitOffset
	return getFieldIterator(bitOffset) == m_fields.end();
}

bool Structure::areEmptyFieldsInBytes(int offset, int size) {
	return areEmptyFields(offset * 0x8, size * 0x8);
}

Structure::Field* Structure::getField(int bitOffset) {
	auto it = getFieldIterator(bitOffset);
	if (it != m_fields.end()) {
		return it->second;
	}
	return getDefaultField();
}

void Structure::addField(int offset, const std::string& name, DataTypePtr type, const std::string& desc) {
	addField(offset * 0x8, type->getSize() * 0x8, name, type, desc);
}

void Structure::addField(int bitOffset, int bitSize, const std::string& name, DataTypePtr type, const std::string& comment) {
	auto factory = getTypeManager()->getProject()->getSymbolManager()->getFactory();
	auto field = factory.createStructFieldSymbol(bitOffset, bitSize, this, type, name, comment);
	addField(field);
}

void Structure::addField(Field* field) {
	field->setStructure(this);
	m_fields.insert(std::make_pair(field->getAbsBitOffset(), field));
	m_size = getSizeByLastField();
}

bool Structure::removeField(Field* field) {
	return removeField(field->getAbsBitOffset());
}

bool Structure::removeField(int bitOffset) {
	auto it = getFieldIterator(bitOffset);
	if (it != m_fields.end()) {
		m_fields.erase(it);
		return true;
	}
	return false;
}

bool Structure::moveField(int bitOffset, int bitsCount) {
	auto it = getFieldIterator(bitOffset);
	if (it == m_fields.end())
		return false;
	auto field = it->second;

	if (bitsCount > 0) {
		if (!areEmptyFields(field->getAbsBitOffset() + field->getBitSize(), std::abs(bitsCount)))
			return false;
	}
	else {
		if (!areEmptyFields(field->getAbsBitOffset() - std::abs(bitsCount), std::abs(bitsCount)))
			return false;
	}

	moveField_(field->getAbsBitOffset(), bitsCount);
	field->getAbsBitOffset() += bitsCount;
	return true;
}

bool Structure::moveFields(int bitOffset, int bitsCount) {
	int firstBitOffset = bitOffset;
	int lastBitOffset = m_size * 0x8 - 1;
	if (!areEmptyFields((bitsCount > 0 ? lastBitOffset : firstBitOffset) - std::abs(bitsCount), std::abs(bitsCount)))
		return false;

	auto it = getFieldIterator(firstBitOffset);
	auto end = m_fields.end();
	if (bitsCount > 0) {
		end--;
		it--;
		std::swap(it, end);
	}
	while (it != end) {
		auto field = it->second;
		moveField_(field->getAbsBitOffset(), bitsCount);
		field->getAbsBitOffset() += bitsCount;
		if (bitsCount > 0)
			it--; else it++;
	}
	return true;
}

Structure::FieldMapType::iterator Structure::getFieldIterator(int bitOffset) {
	auto it = std::prev(m_fields.upper_bound(bitOffset));
	if (it != m_fields.end()) {
		auto field = it->second;
		if (bitOffset < field->getAbsBitOffset() + field->getBitSize()) {
			return it;
		}
	}
	return m_fields.end();
}

Structure::Field* Structure::getDefaultField() {
	return m_defaultField;
}

void Structure::moveField_(int bitOffset, int bitsCount) {
	auto field_ = m_fields.extract(bitOffset);
	field_.key() += bitsCount;
	m_fields.insert(std::move(field_));
}
