#include "Typedef.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace CE::DataType;

CE::DataType::Typedef::Typedef(TypeManager* typeManager, const std::string& name, const std::string& comment)
	: UserDefinedType(typeManager, name, comment)
{
	m_refType = GetUnit(typeManager->getFactory().getDefaultType());
}

Typedef::Group Typedef::getGroup() {
	return Group::Typedef;
}

int Typedef::getSize() {
	if (getRefType()->getType() == this)
		return 0;
	return getRefType()->getSize();
}

std::string DataType::Typedef::getViewValue(uint64_t value) {
	if (getRefType()->getType() == this)
		return UserDefinedType::getViewValue(value);
	return getRefType()->getViewValue(value);
}

void Typedef::setRefType(DataTypePtr refType) {
	
	if (const auto refTypeDef = dynamic_cast<Typedef*>(refType->getType())) {
		if (refTypeDef == this)
			return;
	}
	m_refType = refType;
}

DataTypePtr Typedef::getRefType() const
{
	return m_refType;
}
