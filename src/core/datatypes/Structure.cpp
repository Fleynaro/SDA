#include "Structure.h"
#include <managers/TypeManager.h>
#include <managers/SymbolManager.h>

using namespace CE;
using namespace DataType;

IStructure::Field IStructure::FieldMapType::createField(int absBitOffset, int bitSize, int dataTypeSize,
                                                        const std::string& name, const std::string& comment) const {
	return createField(absBitOffset, bitSize, m_structure->getTypeManager()->getDefaultType(dataTypeSize), name,
	                   comment);
}

Structure::Structure(TypeManager* typeManager, const std::string& name, const std::string& comment)
	: UserDefinedType(typeManager, name, comment), m_fields(this, 0)
{
	const auto factory = typeManager->getProject()->getSymbolManager()->getFactory(false);
	m_defaultField = factory.createStructFieldSymbol(-1, -1, this, GetUnit(typeManager->getFactory().getDefaultType()), "undefined");
	m_defaultField->m_isDefault = true;
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
	return m_fields.getSize();
}

void Structure::resize(int size) {
	m_fields.setSize(size);
}

Structure::FieldMapType& Structure::getFields() {
	return m_fields;
}

void Structure::setFields(const FieldMapType& fields) {
	m_fields = fields;
}

Structure::Field* Structure::getField(int bitOffset) {
	const auto it = m_fields.getFieldIterator(bitOffset);
	if (it != m_fields.end()) {
		return it->second;
	}
	return getDefaultField();
}

void Structure::addField(int offset, const std::string& name, DataTypePtr type, const std::string& desc) {
	addField(offset * 0x8, type->getSize() * 0x8, name, type, desc);
}

void Structure::addField(int bitOffset, int bitSize, const std::string& name, DataTypePtr type, const std::string& comment) {
	const auto factory = getTypeManager()->getProject()->getSymbolManager()->getFactory();
	const auto field = factory.createStructFieldSymbol(bitOffset, bitSize, this, type, name, comment);
	addField(field);
}

void Structure::addField(Field* field) {
	field->setStructure(this);
	m_fields.insert(std::make_pair(field->getAbsBitOffset(), field));
	m_fields.setSize(m_fields.getSizeByLastField());
}

bool Structure::removeField(Field* field) {
	return removeField(field->getAbsBitOffset());
}

bool Structure::removeField(int bitOffset) {
	const auto it = m_fields.getFieldIterator(bitOffset);
	if (it != m_fields.end()) {
		m_fields.erase(it);
		return true;
	}
	return false;
}

void Structure::removeAllFields() {
	for(const auto& [offset, fieldSymbol] : m_fields) {
		// todo: remove symbol
	}
	m_fields.clear();
}

Structure::Field* Structure::getDefaultField() const
{
	return m_defaultField;
}

void Structure::moveField_(int bitOffset, int bitsCount) {
	auto field_ = m_fields.extract(bitOffset);
	field_.key() += bitsCount;
	m_fields.insert(std::move(field_));
}
