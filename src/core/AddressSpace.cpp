#include "AddressSpace.h"
#include <managers/AddressSpaceManager.h>

fs::path CE::AddressSpace::getImagesDirectory() {
	return getAddrSpaceManager()->getProject()->getImagesDirectory() / fs::path(getName());
}

std::map<std::uintptr_t, CE::ImageDecorator*>& CE::AddressSpace::getImages()
{
	return m_images;
}

CE::ImageDecorator* CE::AddressSpace::getImageAt(std::uintptr_t addr)
{
	auto it = std::prev(m_images.upper_bound(addr));
	if (it != m_images.end())
	{
		auto offset = it->first;
		auto image = it->second;
		if (addr < offset + image->getSize())
		{
			return image;
		}
	}
	throw ImageNotFound();
}

CE::AddressSpaceManager* CE::AddressSpace::getAddrSpaceManager() const
{
	return m_addrSpaceManager;
}
