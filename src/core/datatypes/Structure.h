#pragma once
#include "UserType.h"
#include <symbols/StructFieldSymbol.h>
#include <set>
#include <map>

namespace CE::DataType
{
	class IStructure : virtual public IUserDefinedType
	{
	public:
		using Field = Symbol::StructFieldSymbol;
		class FieldMapType : public std::map<int, Field*>
		{
			IStructure* m_structure;
		public:
			FieldMapType(IStructure* structure = nullptr)
				: m_structure(structure)
			{}
			
			int getNextEmptyBitsCount(int bitOffset) {
				const auto it = upper_bound(bitOffset);
				if (it != end()) {
					return it->first - bitOffset;
				}
				return m_structure->getSize() - bitOffset;
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

			bool areEmptyFieldsInBytes(int offset, int size) {
				return areEmptyFields(offset * 0x8, size * 0x8);
			}
			
			iterator getFieldIterator(int bitOffset) {
				const auto it = std::prev(upper_bound(bitOffset));
				if (it != end()) {
					const auto field = it->second;
					if (bitOffset < field->getAbsBitOffset() + field->getBitSize()) {
						return it;
					}
				}
				return end();
			}
		};

		virtual void resize(int size) = 0;

		virtual int getSizeByLastField() = 0;

		virtual FieldMapType& getFields() = 0;

		virtual void setFields(const FieldMapType& fields) = 0;

		// todo: move all below to FieldMapType
		virtual Field* getField(int bitOffset) = 0;

		// todo: remove
		virtual void addField(int bitOffset, int bitSize, const std::string& name, DataTypePtr type, const std::string& desc = "") = 0;

		// todo: remove
		virtual void addField(int offset, const std::string& name, DataTypePtr type, const std::string& desc = "") = 0;

		virtual void addField(Field* field) = 0;

		virtual bool removeField(Field* field) = 0;

		virtual bool removeField(int bitOffset) = 0;

		virtual void removeAllFields() = 0;
	};

	class Structure : public UserDefinedType, public IStructure
	{
	public:
		Structure(TypeManager* typeManager, const std::string& name, const std::string& comment = "");

		~Structure();

		Group getGroup() override;

		int getSize() override;

		void resize(int size) override;

		int getSizeByLastField() override;

		FieldMapType& getFields() override;

		void setFields(const FieldMapType& fields) override;

		Field* getField(int bitOffset) override;

		void addField(int bitOffset, int bitSize, const std::string& name, DataTypePtr type, const std::string& desc = "") override;

		void addField(int offset, const std::string& name, DataTypePtr type, const std::string& desc = "") override;

		void addField(Field* field) override;

		bool removeField(Field* field) override;

		bool removeField(int bitOffset) override;

		void removeAllFields() override;

	private:
		Field* getDefaultField() const;

		void moveField_(int bitOffset, int bitsCount);

	protected:
		int m_size = 0;
		FieldMapType m_fields;
		Field* m_defaultField;
	};
};