#include "AbstractType.h"
#include "Type.h"
#include <Decompiler/DecMask.h>

using namespace CE;
using namespace CE::DataType;

std::string AbstractType::getViewValue(uint64_t value) {
	return std::to_string(value & Decompiler::BitMask64(getSize()).getValue());
}

IType* AbstractType::getBaseType(bool refType, bool dereferencedType) {
	if (auto unit = dynamic_cast<DataType::Unit*>(this)) {
		return unit->getType()->getBaseType();
	}
	if (refType) {
		if (auto typeDef = dynamic_cast<DataType::Typedef*>(this)) {
			return typeDef->getRefType()->getBaseType();
		}
	}
	return this;
}

bool AbstractType::isSystem() {
	return !isUserDefined();
}

bool AbstractType::isSigned() {
	return false;
}

void AbstractType::setTypeManager(TypeManager* typeManager) {
	m_typeManager = typeManager;
}

TypeManager* AbstractType::getTypeManager() {
	return m_typeManager;
}

