#include "AbstractType.h"
#include "TypeUnit.h"
#include "Typedef.h"

using namespace CE;
using namespace DataType;

IType* AbstractType::getBaseType(bool refType, bool dereferencedType) {
	if (const auto unit = dynamic_cast<Unit*>(this)) {
		return unit->getType()->getBaseType();
	}
	if (refType) {
		if (const auto typeDef = dynamic_cast<DataType::Typedef*>(this)) {
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

