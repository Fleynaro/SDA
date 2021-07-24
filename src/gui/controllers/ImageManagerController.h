#pragma once
#include "AbstractManagerController.h"
#include "AddressSpace.h"
#include "managers/AddressSpaceManager.h"
#include "managers/ImageManager.h"


namespace GUI
{
	class ImageManagerController : public AbstractManagerController<CE::ImageDecorator, CE::ImageManager>
	{
	public:
		struct ImageFilter
		{
			std::string m_name;
		};

		class ImageListModel : public ListModel
		{
		public:
			bool m_isTable;
			ImageListModel(ImageManagerController* controller, bool isTable)
				: ListModel(controller), m_isTable(isTable)
			{}

		private:
			class ImageIterator : public Iterator
			{
			public:
				using Iterator::Iterator;

			private:
				std::string getText(CE::ImageDecorator* item) override
				{
					if(static_cast<ImageListModel*>(m_listModel)->m_isTable)
						return item->getAddressSpace()->getName() + "," + item->getName();
					return item->getName();
				}
			};

			void newIterator(const IteratorCallback& callback) override
			{
				ImageIterator iterator(this);
				callback(&iterator);
			}
		};

		ImageFilter m_filter;
		ImageListModel m_tableListModel;
		ImageListModel m_listModel;

		ImageManagerController(CE::ImageManager* manager)
			: AbstractManagerController<CE::ImageDecorator, CE::ImageManager>(manager), m_tableListModel(this, true), m_listModel(this, false)
		{}

		CE::ImageDecorator* createImage(CE::AddressSpace* addrSpace, const std::string& name, const fs::path& pathToImage) const {
			return m_manager->createImage(addrSpace, CE::ImageDecorator::IMAGE_PE, name);
		}
		
		CE::AddressSpace* createAddrSpace(const std::string& name) const {
			return getAddressSpaceManager()->createAddressSpace(name);
		}

		CE::AddressSpaceManager* getAddressSpaceManager() const {
			return m_manager->getProject()->getAddrSpaceManager();
		}

	private:
		bool filter(CE::ImageDecorator* item) override
		{
			using namespace Helper::String;
			if (!m_filter.m_name.empty() && ToLower(item->getName()).find(ToLower(m_filter.m_name)) == std::string::npos)
				return false;

			return true;
		}
	};
};
