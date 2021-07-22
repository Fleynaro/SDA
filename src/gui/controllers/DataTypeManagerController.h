#pragma once
#include "AbstractManagerController.h"
#include "managers/TypeManager.h"

namespace GUI
{
	class DataTypeManagerController : public AbstractManagerController<CE::DataType::IType>
	{
	public:
		struct DataTypeFilter
		{
			std::string m_name;
		};

		class DataTypeListModel : public ListModel
		{
		public:
			using ListModel::ListModel;

		private:
			class DataTypeIterator : public Iterator
			{
			public:
				using Iterator::Iterator;

			private:
				std::string getText(CE::DataType::IType* item) override
				{
					std::string groupName = "Unknown";
					switch (item->getGroup()) {
					case CE::DataType::IType::Simple:
						groupName = "Simple";
						break;
					case CE::DataType::IType::Typedef:
						groupName = "Typedef";
						break;
					case CE::DataType::IType::Enum:
						groupName = "Enum";
						break;
					case CE::DataType::IType::Structure:
						groupName = "Structure";
						break;
					case CE::DataType::IType::Class:
						groupName = "Class";
						break;
					case CE::DataType::IType::FunctionSignature:
						groupName = "FunctionSignature";
						break;
					}
					return item->getName() + "," + groupName;
				}
			};

			void newIterator(const IteratorCallback& callback) override
			{
				DataTypeIterator iterator(this);
				callback(&iterator);
			}
		};

		DataTypeFilter m_filter;
		DataTypeListModel m_listModel;

		DataTypeManagerController(CE::TypeManager* manager)
			: AbstractManagerController<CE::DataType::IType>(manager), m_listModel(this)
		{}

	private:
		bool filter(CE::DataType::IType* item) override
		{
			using namespace Helper::String;
			if (!m_filter.m_name.empty() && ToLower(item->getName()).find(ToLower(m_filter.m_name)) == std::string::npos)
				return false;
			return true;
		}
	};
};
