#include "DomainObject.h"
#include <stdexcept>

DB::DomainObject::DomainObject(Id id)
	: m_id(id)
{}

DB::Id DB::DomainObject::getId() {
	if (m_id == 0)
		throw std::logic_error("id = 0");
	return m_id;
}

void DB::DomainObject::setId(Id id) {
	m_id = id;
}

bool DB::DomainObject::isInDB() {
	return m_id != 0;
}

DB::IMapper* DB::DomainObject::getMapper() {
	return m_mapper;
}

void DB::DomainObject::setMapper(IMapper* mapper) {
	m_mapper = mapper;
}
