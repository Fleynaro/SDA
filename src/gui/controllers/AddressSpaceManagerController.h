#pragma once
#include "AbstractManagerController.h"
#include "managers/AddressSpaceManager.h"

namespace GUI
{
	class AddressSpaceManagerController : public AbstractManagerController<CE::AddressSpace>
	{
	public:
		struct AddressSpaceFilter
		{
			std::string m_name;
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
				AddressSpaceIterator iterator(m_controller);
				callback(&iterator);
			}
		};

		AddressSpaceFilter m_filter;
		AddressSpaceListModel m_listModel;

		AddressSpaceManagerController(CE::AddressSpaceManager* manager)
			: AbstractManagerController<CE::AddressSpace>(manager), m_listModel(this)
		{}

	private:
		bool filter(CE::AddressSpace* item) override
		{
			using namespace Helper::String;
			if (!m_filter.m_name.empty() && ToLower(item->getName()).find(ToLower(m_filter.m_name)) == std::string::npos)
				return false;

			return true;
		}
	};
};
