#include "Structure.h"
#include <managers/TypeManager.h>
#include <managers/SymbolManager.h>

using namespace CE;
using namespace DataType;

void FieldList::addField(int bitOffset, int bitSize, const std::string& name, DataTypePtr type,
                            const std::string& comment) {
	const auto factory = m_structure->getTypeManager()->getProject()->getSymbolManager()->getFactory();
	const auto field = factory.createStructFieldSymbol(bitSize, type, name, comment);
	addField(bitOffset, field);
}

Symbol::StructFieldSymbol FieldList::createField(int absBitOffset, int bitSize, int dataTypeSize,
                                                    const std::string& name, const std::string& comment) const {
	return createField(absBitOffset, bitSize, m_structure->getTypeManager()->getDefaultType(dataTypeSize), name,
	                   comment);
}

Structure::Structure(TypeManager* typeManager, const std::string& name, const std::string& comment)
	: UserDefinedType(typeManager, name, comment), m_fields(this)
{}

Structure::~Structure() {
	for (auto const& [offset, field] : m_fields)
		delete field;
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

FieldList& Structure::getFields() {
	return m_fields;
}
