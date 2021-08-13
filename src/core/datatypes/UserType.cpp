#include "UserType.h"
#include <utilities/ObjectHash.h>

using namespace CE;
using namespace DataType;

bool UserDefinedType::isUserDefined() {
	return true;
}

std::string UserDefinedType::getDisplayName() {
	return getName();
}