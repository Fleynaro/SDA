#include "GhidraObject.h"

using namespace CE;
using namespace Ghidra;

bool Object::doesSyncWithGhidra() {
	return m_sync;
}

void Object::makeSyncWithGhidra(bool toggle) {
	m_sync = toggle;
}

IMapper* Object::getGhidraMapper() {
	return m_mapper;
}

void Object::setGhidraMapper(IMapper* mapper) {
	m_mapper = mapper;
}
