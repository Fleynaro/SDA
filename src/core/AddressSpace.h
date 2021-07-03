#pragma once
#include <ImageDecorator.h>

namespace CE
{
	class AddressSpaceManager;

	class AddressSpace : public DB::DomainObject, public Description
	{
		// exceptions
		class ImageNotFound : public std::exception
		{
		public:
			ImageNotFound()
				: std::exception("image not found")
			{}
		};

		AddressSpaceManager* m_addrSpaceManager;
		std::map<std::uintptr_t, ImageDecorator*> m_images;
	public:
		AddressSpace(AddressSpaceManager* addrSpaceManager, const std::string& name, const std::string& comment = "")
			: m_addrSpaceManager(addrSpaceManager), Description(name, comment)
		{}

		fs::path getImagesDirectory();

		auto& getImages() {
			return m_images;
		}
		
		ImageDecorator* getImageAt(std::uintptr_t addr) {
			auto it = std::prev(m_images.upper_bound(addr));
			if (it != m_images.end()) {
				auto offset = it->first;
				auto image = it->second;
				if (addr < offset + image->getSize()) {
					return image;
				}
			}
			throw ImageNotFound();
		}

		AddressSpaceManager* getAddrSpaceManager() const
		{
			return m_addrSpaceManager;
		}
	};
};