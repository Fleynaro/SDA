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

		std::map<std::uintptr_t, ImageDecorator*>& getImages();

		ImageDecorator* getImageAt(std::uintptr_t addr);

		AddressSpaceManager* getAddrSpaceManager() const;
	};
};