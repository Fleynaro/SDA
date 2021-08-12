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
		const auto image = imageDec->getImage();
		if(addr >= image->getAddress() && addr < image->getAddress() + image->getSize()) {
			return imageDec;
		}
	}
	return nullptr;
}

CE::AddressSpaceManager* CE::AddressSpace::getAddrSpaceManager() const
{
	return m_addrSpaceManager;
}
