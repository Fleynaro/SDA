#pragma once
#include "AbstractManagerController.h"
#include "managers/AddressSpaceManager.h"

namespace GUI
{
	class AddressSpaceController
	{
	public:
		CE::AddressSpace* m_addrSpace;
		
		AddressSpaceController(CE::AddressSpace* addrSpace)
			: m_addrSpace(addrSpace)
		{}

		void rename(const std::string& name) const {
			m_addrSpace->setName(name);
			m_addrSpace->getAddrSpaceManager()->getProject()->getTransaction()->markAsDirty(m_addrSpace);
		}
	};
	
	class AddressSpaceManagerController : public AbstractManagerController<CE::AddressSpace, CE::AddressSpaceManager>
	{
	public:
		struct AddressSpaceFilter
		{
			std::string m_name;
			bool m_showEmpty = false;
		};

		class AddressSpaceListModel : public ListModel
		{
		public:
			using ListModel::ListModel;

		private:
			class AddressSpaceIterator : public Iterator
			{
			public:
				using Iterator::Iterator;

			private:
				std::string getText(CE::AddressSpace* item) override
				{
					return item->getName();
				}
			};

			void newIterator(const IteratorCallback& callback) override
			{
				AddressSpaceIterator iterator(this);
				callback(&iterator);
			}
		};

		AddressSpaceFilter m_filter;
		AddressSpaceListModel m_listModel;
		
		AddressSpaceManagerController(CE::AddressSpaceManager* manager)
			: AbstractManagerController<CE::AddressSpace, CE::AddressSpaceManager>(manager), m_listModel(this)
		{}

		CE::AddressSpace* createAddrSpace(const std::string& name) const {
			return m_manager->createAddressSpace(name);
		}
	
	private:
		bool filter(CE::AddressSpace* item) override
		{
			using namespace Helper::String;
			if (!m_filter.m_name.empty() && ToLower(item->getName()).find(ToLower(m_filter.m_name)) == std::string::npos)
				return false;
			if (m_filter.m_showEmpty && !item->getImageDecorators().empty())
				return false;
			return true;
		}
	};
};
