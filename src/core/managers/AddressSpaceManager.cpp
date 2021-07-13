#include "AddressSpaceManager.h"
#include <database/Mappers/AddressSpaceMapper.h>
#include <filesystem>
namespace fs = std::filesystem;

using namespace CE;

AddressSpaceManager::AddressSpaceManager(Project* project)
	: AbstractItemManager(project)
{
	m_imageMapper = new DB::AddressSpaceMapper(this);
}

AddressSpace* AddressSpaceManager::createAddressSpace(const std::string& name, const std::string& desc, bool markAsNew) {
	auto addressSpace = new AddressSpace(this, name, desc);
	if (!exists(addressSpace->getImagesDirectory()))
		create_directory(addressSpace->getImagesDirectory());

	addressSpace->setMapper(m_imageMapper);
	if (markAsNew) {
		getProject()->getTransaction()->markAsNew(addressSpace);
	}
	return addressSpace;
}

void AddressSpaceManager::loadAddressSpaces() const
{
	m_imageMapper->loadAll();
}

AddressSpace* AddressSpaceManager::findAddressSpaceById(DB::Id id) {
	return dynamic_cast<AddressSpace*>(find(id));
}

AddressSpace* AddressSpaceManager::findAddressSpaceByName(const std::string& name) {
	Iterator it(this);
	while (it.hasNext()) {
		auto item = it.next();
		if (item->getName() == name) {
			return item;
		}
	}
	throw ItemNotFoundException();
}
