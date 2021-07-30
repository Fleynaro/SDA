#pragma once
#include "UserType.h"
#include <symbols/StructFieldSymbol.h>
#include <set>
#include <map>

namespace CE::DataType
{
	class FieldList
	{
		std::map<int, Symbol::StructFieldSymbol*> m_fields;
		int m_size = 0;
		using iterator = std::map<int, Symbol::StructFieldSymbol*>::iterator;
	public:
		IStructure* m_structure;
		
		FieldList(IStructure* structure)
			: m_structure(structure)
		{}

		Symbol::StructFieldSymbol* operator[](int bitOffset) {
			const auto it = getFieldIterator(bitOffset);
			if (it != end())
				return it->second;
			return nullptr;
		}

		Symbol::StructFieldSymbol* getLastField() {
			if (m_fields.empty())
				return nullptr;
			return m_fields.rbegin()->second;
		}

		int getNextEmptyBitsCount(int bitOffset) {
			const auto it = m_fields.upper_bound(bitOffset);
			if (it != end()) {
				return it->first - bitOffset;
			}
			return getSize() * 0x8 - bitOffset;
		}

		bool areEmptyFields(int bitOffset, int bitSize) {
			if (bitOffset < 0 || bitSize <= 0)
				return false;

			//check free space to the next field starting at the bitOffset
			if (getNextEmptyBitsCount(bitOffset) < bitSize)
				return false;

			//check intersecting with an existing field at the bitOffset
			return getFieldIterator(bitOffset) == end();
		}

		int getFieldsCount() const {
			return static_cast<int>(m_fields.size());
		}

		iterator begin() {
			return m_fields.begin();
		}

		iterator end() {
			return m_fields.end();
		}

		int getSizeByLastField() {
			const auto lastField = getLastField();
			if (!lastField)
				return 0;
			return lastField->getOffset() + lastField->getSize();
		}

		int getSize() const {
			return m_size;
		}

		void setSize(int size) {
			m_size = size;
		}

		void addField(int bitOffset, Symbol::StructFieldSymbol* field) {
			m_fields[bitOffset] = field;
			field->setAbsBitOffset(bitOffset);
			field->m_structure = m_structure;
			setSize(getSizeByLastField());
		}

		void addField(int offset, const std::string& name, DataTypePtr type, const std::string& comment = "") {
			addField(offset * 0x8, type->getSize() * 0x8, name, type, comment);
		}

		void addField(int bitOffset, int bitSize, const std::string& name, DataTypePtr type,
		              const std::string& comment = "");

		void removeField(int bitOffset) {
			m_fields.erase(bitOffset);
		}

		void clear() {
			m_fields.clear();
		}

		bool moveField(Symbol::StructFieldSymbol* field, int delta) {
			const auto newAbsBitOffset = field->getAbsBitOffset() + delta;
			m_fields.erase(field->getAbsBitOffset());
			if (!areEmptyFields(newAbsBitOffset, field->getBitSize())) {
				m_fields[field->getAbsBitOffset()] = field;
				return false;
			}
			m_fields[newAbsBitOffset] = field;
			field->setAbsBitOffset(newAbsBitOffset);
			return true;
		}
		
		Symbol::StructFieldSymbol createField(int absBitOffset, int bitSize, DataTypePtr dataType, const std::string& name, const std::string& comment = "") const {
			auto field = Symbol::StructFieldSymbol(nullptr, dataType, bitSize, name, comment);
			field.setAbsBitOffset(absBitOffset);
			field.m_structure = m_structure;
			return field;
		}

		Symbol::StructFieldSymbol createField(int absBitOffset, int bitSize, int dataTypeSize, const std::string& name,
			const std::string& comment = "") const;

	private:
		iterator getFieldIterator(int bitOffset) {
			const auto it = std::prev(m_fields.upper_bound(bitOffset));
			if (it != m_fields.end()) {
				const auto field = it->second;
				if (bitOffset < field->getAbsBitOffset() + field->getBitSize()) {
					return it;
				}
			}
			return m_fields.end();
		}
	};
	
	class IStructure : virtual public IUserDefinedType
	{
	public:
		virtual void resize(int size) = 0;

		virtual FieldList& getFields() = 0;

		virtual IStructure* clone() = 0;

		virtual void apply(IStructure* structure) = 0;
	};

	class Structure : public UserDefinedType, public IStructure
	{
	public:
		Structure(TypeManager* typeManager, const std::string& name, const std::string& comment = "");

		~Structure();

		Group getGroup() override;

		int getSize() override;

		void resize(int size) override;

		FieldList& getFields() override;

		IStructure* clone() override {
			const auto structure = new Structure(getTypeManager(), getName(), getComment());
			for (const auto& [offset, field] : m_fields) {
				structure->getFields().addField(offset, field->clone());
			}
			structure->getFields().setSize(m_fields.getSize());
			return structure;
		}

		void apply(IStructure* structure) override {
			m_fields = structure->getFields();
			structure->getFields().clear();
			m_fields.m_structure = this;
			for (const auto& [offset, field] : m_fields) {
				field->m_structure = this;
			}
		}
	
	protected:
		FieldList m_fields;
	};
};