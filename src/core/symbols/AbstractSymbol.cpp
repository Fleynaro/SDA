#include "AbstractSymbol.h"

using namespace CE;
using namespace CE::Symbol;

void AbstractSymbol::setAutoSymbol(bool toggle) {
	m_isAutoSymbol = toggle;
}

bool AbstractSymbol::isAutoSymbol() {
	return m_isAutoSymbol;
}

SymbolManager* AbstractSymbol::getManager() {
	return m_manager;
}

DataTypePtr AbstractSymbol::getDataType() {
	return m_dataType;
}

void AbstractSymbol::setDataType(DataTypePtr dataType) {
	m_dataType = dataType;
}
