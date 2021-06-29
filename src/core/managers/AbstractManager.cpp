#include "AbstractManager.h"

using namespace CE;

AbstractItemManager::AbstractItemManager(Project* programModule)
	: AbstractManager(programModule)
{}

void AbstractItemManager::onLoaded(DB::IDomainObject* obj) {
	m_items.insert(std::make_pair(obj->getId(), obj));
}

void AbstractItemManager::onChangeBeforeCommit(DB::IDomainObject* obj, ChangeType type) {
	switch (type)
	{
	case Inserted:
		if (m_items.find(obj->getId()) != m_items.end())
			throw std::exception("item has been already in the manager");
		m_items.insert(std::make_pair(obj->getId(), obj));
		break;
	case Removed:
		m_items.erase(obj->getId());
		break;
	}
}

void AbstractItemManager::onChangeAfterCommit(DB::IDomainObject* obj, ChangeType type) {
}

DB::IDomainObject* AbstractItemManager::find(DB::Id id) {
	if (m_items.find(id) == m_items.end())
		throw ItemNotFoundException();
	return m_items[id];
}

int AbstractItemManager::getItemsCount() {
	return (int)m_items.size();
}

AbstractManager::AbstractManager(Project* project)
	: m_project(project)
{}

Project* AbstractManager::getProject() {
	return m_project;
}
