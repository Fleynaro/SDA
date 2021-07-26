#pragma once
#include "AbstractManagerController.h"
#include "managers/TypeManager.h"

namespace GUI
{
	static std::string GetGroupName(CE::DataType::IType* dataType) {
		std::string groupName = "Unknown";
		switch (dataType->getGroup()) {
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
			groupName = "Function signature";
			break;
		}
		return groupName;
	}
	
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
			bool m_isTable;
			DataTypeListModel(DataTypeManagerController* controller, bool isTable)
				: ListModel(controller), m_isTable(isTable)
			{}

		private:
			class DataTypeIterator : public Iterator
			{
			public:
				using Iterator::Iterator;

			private:
				std::string getText(CE::DataType::IType* item) override
				{
					if (dynamic_cast<DataTypeListModel*>(m_listModel)->m_isTable)
						return item->getName() + "," + GetGroupName(item);
					return item->getName();
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
		DataTypeListModel m_tableListModel;

		DataTypeManagerController(CE::TypeManager* manager)
			: AbstractManagerController<CE::DataType::IType>(manager), m_listModel(this, false), m_tableListModel(this, true)
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
