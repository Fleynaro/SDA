#include "AddressSpace.h"
#include <managers/AddressSpaceManager.h>

fs::path CE::AddressSpace::getImagesDirectory() {
	return getAddrSpaceManager()->getProject()->getImagesDirectory() / fs::path(getName());
}

std::list<CE::ImageDecorator*>& CE::AddressSpace::getImageDecorators()
{
	return m_imagesDecorators;
}

CE::ImageDecorator* CE::AddressSpace::getImageDecoratorAt(std::uintptr_t addr)
{
	for(const auto imageDec : m_imagesDecorators) {
		if(addr >= imageDec->getImage()->getAddress() && addr < imageDec->getImage()->getAddress() + imageDec->getImage()->getSize()) {
			return imageDec;
		}
	}
	return nullptr;
}

CE::AddressSpaceManager* CE::AddressSpace::getAddrSpaceManager() const
{
	return m_addrSpaceManager;
}
