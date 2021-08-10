#pragma once
#include "AbstractManager.h"
#include <AddressSpace.h>

namespace DB {
	class AddressSpaceMapper;
};

namespace CE
{
	class AddressSpaceManager : public AbstractItemManager
	{
	public:
		using Iterator = AbstractIterator<AddressSpace>;

		AddressSpace* m_debugAddressSpace = nullptr;

		AddressSpaceManager(Project* project);

		AddressSpace* createAddressSpace(const std::string& name, const std::string& desc = "", bool markAsNew = true);

		void loadAddressSpaces() const;

		AddressSpace* findAddressSpaceById(DB::Id id);

		AddressSpace* findAddressSpaceByName(const std::string& name);

	private:
		DB::AddressSpaceMapper* m_imageMapper;
	};
};