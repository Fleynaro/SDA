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

	class EnumFieldListModel : public IListModel<int>
	{
		CE::DataType::Enum::FieldMapType* m_fields;
	public:
		class FieldIterator : public Iterator
		{
			CE::DataType::Enum::FieldMapType::iterator m_it;
			CE::DataType::Enum::FieldMapType* m_fields;
		public:
			FieldIterator(CE::DataType::Enum::FieldMapType* fields)
				: m_fields(fields), m_it(fields->begin())
			{}

			void getNextItem(std::string* text, int* data) override {
				const auto& [offset, name] = *m_it;
				*text = name + "," + std::to_string(offset);
				*data = m_it->first;
				++m_it;
			}

			bool hasNextItem() override {
				return m_it != m_fields->end();
			}
		};

		EnumFieldListModel(CE::DataType::Enum::FieldMapType* fields)
			: m_fields(fields)
		{}

		void newIterator(const IteratorCallback& callback) override
		{
			FieldIterator iterator(m_fields);
			callback(&iterator);
		}
	};

	class StructureFieldListModel : public IListModel<CE::Symbol::StructFieldSymbol*>
	{
		CE::DataType::FieldList* m_fields;
	public:
		class FieldIterator : public Iterator
		{
			StructureFieldListModel* m_model;
			CE::DataType::FieldList* m_fields;
			int m_bitOffset = 0;
			CE::DataTypePtr m_bitFieldDataType;
			int m_bitFieldEndOffset = 0;
			CE::Symbol::StructFieldSymbol m_emptyField;
		public:
			FieldIterator(StructureFieldListModel* model)
				: m_model(model), m_fields(model->m_fields)
			{}

			void getNextItem(std::string* text, CE::Symbol::StructFieldSymbol** data) override {
				if (m_bitOffset >= m_bitFieldEndOffset) {
					m_bitFieldEndOffset = 0;
				}
				
				const auto field = (*m_fields)[m_bitOffset]; // todo: slow
				if(field) {
					if (field->isBitField()) {
						if (!m_bitFieldEndOffset) {
							m_bitFieldDataType = field->getDataType();
							m_bitFieldEndOffset = m_bitOffset + field->getSize() * 0x8;
						}
					}
					*text = getText(field);
					*data = field;
					m_bitOffset += field->getBitSize();
				} else {
					if (m_bitFieldEndOffset) {
						m_emptyField = m_fields->createField(m_bitOffset, 0x1, m_bitFieldDataType, " ");
						m_bitOffset += 1;
					}
					else if (m_fields->getNextEmptyBitsCount(m_bitOffset) < 0x8) { // todo: slow
						m_emptyField = m_fields->createField(m_bitOffset, 0x1, 0x1, " ");
						m_bitOffset += 1;
					}
					else {
						m_emptyField = m_fields->createField(m_bitOffset, 0x8, 1, " ");
						m_bitOffset += 8;
					}
					m_emptyField.m_isDefault = true;
					*text = getText(&m_emptyField);
					*data = &m_emptyField;
				}
			}

			std::string getText(CE::Symbol::StructFieldSymbol* field) {
				using namespace Helper::String;
				auto offsetStr = m_model->m_hexView ? "0x" + NumberToHex(field->getOffset()) : std::to_string(field->getOffset());
				auto sizeStr = m_model->m_hexView ? "0x" + NumberToHex(field->getSize()) : std::to_string(field->getSize());
				if (field->isBitField()) {
					offsetStr += ":" + std::to_string(field->getBitOffset());
					sizeStr += " (" + std::to_string(field->getBitSize()) + " bits)";
				}
				return offsetStr + "," + sizeStr + "," + field->getDataType()->getDisplayName() + "," + field->getName();
			}

			bool hasNextItem() override {
				return m_bitOffset < m_fields->getSize() * 0x8;
			}
		};

		bool m_hexView = false;

		StructureFieldListModel(CE::DataType::FieldList* fields)
			: m_fields(fields)
		{}

		void newIterator(const IteratorCallback& callback) override
		{
			FieldIterator iterator(this);
			callback(&iterator);
		}
	};

	class ParamListModel : public IListModel<CE::Symbol::FuncParameterSymbol*>
	{
		CE::DataType::ParameterList* m_params;
	public:
		class ParamIterator : public Iterator
		{
			int m_paramIdx = 0;
			CE::DataType::ParameterList* m_params;
		public:
			ParamIterator(CE::DataType::ParameterList* params)
				: m_params(params)
			{}

			void getNextItem(std::string* text, CE::Symbol::FuncParameterSymbol** data) override {
				const auto param = (*m_params)[m_paramIdx];
				*text = std::to_string(m_paramIdx + 1) + "," + param->getDataType()->getDisplayName() + "," + param->getName() + "," + getStoragesText(param->getParamInfo());
				*data = param;
				m_paramIdx++;
			}

			bool hasNextItem() override {
				return m_paramIdx < m_params->getParamsCount();
			}

			std::string getStoragesText(const CE::Decompiler::ParameterInfo& paramInfo) const {
				using namespace CE::Decompiler;
				using namespace Helper::String;
				const auto& storage = paramInfo.m_storage;
				std::string text;
				if (storage.getType() == Storage::STORAGE_REGISTER) {
					const auto reg = PCode::Register(storage.getRegGenericId(), 0, paramInfo.m_size);
					text = PCode::InstructionViewGenerator::GenerateRegisterName(reg);
				}
				else {
					text = storage.getType() == Storage::STORAGE_GLOBAL ? "global" : "stack";
					text += " + 0x" + NumberToHex(storage.getOffset());
				}
				return text;
			}
		};

		ParamListModel(CE::DataType::ParameterList* params)
			: m_params(params)
		{}

		void newIterator(const IteratorCallback& callback) override
		{
			ParamIterator iterator(m_params);
			callback(&iterator);
		}
	};
	
	class DataTypeManagerController : public AbstractManagerController<CE::DataType::IType, CE::TypeManager>
	{
	public:
		struct DataTypeFilter
		{
			std::string m_name;
			std::set<CE::DataType::AbstractType::Group> m_groups;
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
						return std::string(item->getName()) + "," + GetGroupName(item);
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
			if (!m_filter.m_groups.empty() && m_filter.m_groups.find(item->getGroup()) == m_filter.m_groups.end())
				return false;
			using namespace Helper::String;
			if (!m_filter.m_name.empty() && ToLower(item->getName()).find(ToLower(m_filter.m_name)) == std::string::npos)
				return false;
			return true;
		}
	};
};
