#include "Typedef.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace DataType;

Typedef::Typedef(TypeManager* typeManager, const std::string& name, const std::string& comment)
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
