#pragma once
#include "AbstractManagerController.h"
#include "managers/ImageManager.h"


namespace GUI
{
	class ImageManagerController : public AbstractManagerController<CE::ImageDecorator>
	{
	public:
		struct ImageFilter
		{
			std::string m_name;
		};

		class ImageListModel : public ListModel
		{
		public:
			using ListModel::ListModel;

		private:
			class ImageIterator : public Iterator
			{
			public:
				using Iterator::Iterator;

			private:
				std::string getText(CE::ImageDecorator* item) override
				{
					return item->getName();
				}
			};

			void newIterator(const IteratorCallback& callback) override
			{
				ImageIterator iterator(m_controller);
				callback(&iterator);
			}
		};

		ImageFilter m_filter;
		ImageListModel m_listModel;

		ImageManagerController(CE::ImageManager* manager)
			: AbstractManagerController<CE::ImageDecorator>(manager), m_listModel(this)
		{}

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
