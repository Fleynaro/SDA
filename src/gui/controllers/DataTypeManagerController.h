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

	// todo: move all such controllers into core (implement undo, redo operations)
	class UserDataTypeController
	{
		CE::DataType::IUserDefinedType* m_dataType;
	protected:
		bool m_changed = false;
	public:
		UserDataTypeController(CE::DataType::IUserDefinedType* dataType)
			: m_dataType(dataType)
		{}

		~UserDataTypeController() {
			if (m_changed) {
				m_dataType->getTypeManager()->getProject()->getTransaction()->markAsDirty(m_dataType);
			}
		}

		void rename(const std::string& name) {
			m_dataType->setName(name);
			m_changed = true;
		}
	};

	class TypedefController : public UserDataTypeController
	{
		CE::DataType::Typedef* m_dataType;
	public:
		TypedefController(CE::DataType::Typedef* dataType)
			: UserDataTypeController(dataType), m_dataType(dataType)
		{}

		void changeRefType(CE::DataTypePtr dataType) {
			m_dataType->setRefType(dataType);
			m_changed = true;
		}
	};

	class EnumController : public UserDataTypeController
	{
		CE::DataType::Enum* m_dataType;
	public:
		EnumController(CE::DataType::Enum* dataType)
			: UserDataTypeController(dataType), m_dataType(dataType)
		{}

		void change(const CE::DataType::Enum::FieldMapType& fields) {
			m_dataType->getFields() = fields;
			m_changed = true;
		}
	};

	class StructureController : public UserDataTypeController
	{
		CE::DataType::IStructure* m_dataType;
		using Field = CE::DataType::IStructure::Field;
	public:
		StructureController(CE::DataType::IStructure* dataType)
			: UserDataTypeController(dataType), m_dataType(dataType)
		{}

		void addField(Field* field) {
			m_dataType->addField(field);
			m_changed = true;
		}

		void removeField(Field* field) {
			m_dataType->removeField(field);
			m_changed = true;
		}

		void removeAllFields() {
			m_dataType->removeAllFields();
			m_changed = true;
		}
	};

	class FunctionSingautreController : public UserDataTypeController
	{
		CE::DataType::IFunctionSignature* m_dataType;
	public:
		FunctionSingautreController(CE::DataType::IFunctionSignature* dataType)
			: UserDataTypeController(dataType), m_dataType(dataType)
		{}

		void addParameter(CE::Symbol::FuncParameterSymbol* symbol) {
			m_dataType->addParameter(symbol);
			m_changed = true;
		}

		void removeLastParameter() {
			m_dataType->removeLastParameter();
			m_changed = true;
		}

		void removeAllParameters() {
			m_dataType->deleteAllParameters();
			m_changed = true;
		}
	};
	
	class DataTypeManagerController : public AbstractManagerController<CE::DataType::IType, CE::TypeManager>
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
			: AbstractManagerController<CE::DataType::IType, CE::TypeManager>(manager), m_listModel(this, false), m_tableListModel(this, true)
		{}

		CE::DataTypePtr parseDataType(const std::string& text) const {
			std::string typeName;
			std::string typePtrLevels;
			bool isTypeName = true;
			for (auto c : text) {
				if (c == '*' || c == '[')
					isTypeName = false;
				if (isTypeName) {
					typeName.push_back(c);
				}
				else {
					typePtrLevels.push_back(c);
				}
			}
			if (const auto dataType = m_manager->findTypeByName(typeName))
				return GetUnit(dataType, typePtrLevels);
			return nullptr;
		}

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
