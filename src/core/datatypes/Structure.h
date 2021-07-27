#pragma once
#include "UserType.h"
#include <symbols/StructFieldSymbol.h>
#include <set>
#include <map>

namespace CE::DataType
{
	class IStructure : public IUserDefinedType
	{
	public:
		using Field = Symbol::StructFieldSymbol;
		using FieldMapType = std::map<int, Field*>;

		virtual void resize(int size) = 0;

		virtual int getSizeByLastField() = 0;

		virtual FieldMapType& getFields() = 0;

		virtual int getNextEmptyBitsCount(int bitOffset) = 0;

		virtual bool areEmptyFields(int bitOffset, int bitSize) = 0;

		virtual bool areEmptyFieldsInBytes(int offset, int size) = 0;

		virtual Field* getField(int bitOffset) = 0;

		// todo: remove
		virtual void addField(int bitOffset, int bitSize, const std::string& name, DataTypePtr type, const std::string& desc = "") = 0;

		// todo: remove
		virtual void addField(int offset, const std::string& name, DataTypePtr type, const std::string& desc = "") = 0;

		virtual void addField(Field* field) = 0;

		virtual bool removeField(Field* field) = 0;

		virtual bool removeField(int bitOffset) = 0;

		virtual void removeAllFields() = 0;

		// todo: remove
		virtual bool moveField(int bitOffset, int bitsCount) = 0;

		// todo: remove
		virtual bool moveFields(int bitOffset, int bitsCount) = 0;
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

		int getNextEmptyBitsCount(int bitOffset) override;

		bool areEmptyFields(int bitOffset, int bitSize) override;

		bool areEmptyFieldsInBytes(int offset, int size) override;

		Field* getField(int bitOffset) override;

		void addField(int bitOffset, int bitSize, const std::string& name, DataTypePtr type, const std::string& desc = "") override;

		void addField(int offset, const std::string& name, DataTypePtr type, const std::string& desc = "") override;

		void addField(Field* field) override;

		bool removeField(Field* field) override;

		bool removeField(int bitOffset) override;

		void removeAllFields() override;

		bool moveField(int bitOffset, int bitsCount) override;

		bool moveFields(int bitOffset, int bitsCount) override;

	private:
		FieldMapType::iterator getFieldIterator(int bitOffset);

		Field* getDefaultField() const;

		void moveField_(int bitOffset, int bitsCount);

	protected:
		int m_size = 0;
		FieldMapType m_fields;
		Field* m_defaultField;
	};
};