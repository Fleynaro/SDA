#pragma once
#include <ImageDecorator.h>

namespace CE
{
	class AddressSpaceManager;

	class AddressSpace : public DB::DomainObject, public Description
	{
		AddressSpaceManager* m_addrSpaceManager;
		std::list<ImageDecorator*> m_imagesDecorators;
	public:
		IDebugSession* m_debugSession = nullptr;
		
		AddressSpace(AddressSpaceManager* addrSpaceManager, const std::string& name, const std::string& comment = "")
			: m_addrSpaceManager(addrSpaceManager), Description(name, comment)
		{}

		fs::path getImagesDirectory();

		std::list<ImageDecorator*>& getImageDecorators();

		ImageDecorator* getImageDecoratorAt(std::uintptr_t addr);

		AddressSpaceManager* getAddrSpaceManager() const;

		bool isDebug() const;
	};
};