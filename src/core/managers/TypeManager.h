#pragma once
#include "AbstractManager.h"
#include <datatypes/DataTypes.h>

namespace DB {
	class DataTypeMapper;
};

namespace CE
{
	class TypeManager : public AbstractItemManager
	{
	public:
		class Factory : public AbstractFactory
		{
			TypeManager* m_typeManager;
			DB::DataTypeMapper* m_dataTypeMapper;
		public:
			Factory(TypeManager* typeManager, DB::DataTypeMapper* dataTypeMapper, bool markAsNew)
				: m_typeManager(typeManager), m_dataTypeMapper(dataTypeMapper), AbstractFactory(markAsNew)
			{}

			DataType::Typedef* createTypedef(const std::string& name, const std::string& desc = "") const;

			DataType::Enum* createEnum(const std::string& name, const std::string& desc = "") const;

			DataType::Structure* createStructure(const std::string& name, const std::string& desc) const;

			DataType::FunctionSignature* createSignature(DataType::CallingConvetion callingConvetion, const std::string& name, const std::string& desc = "") const;

			DataType::FunctionSignature* createSignature(const std::string& name, const std::string& desc = "") const;

			DataType::IType* getDefaultType() const;

			DataType::IType* getDefaultReturnType() const;
		};

		using Iterator = AbstractIterator<DataType::AbstractType>;

		TypeManager(Project* module);

		~TypeManager();

		Factory getFactory(bool markAsNew = true);

		void addSystemTypes();

		void addGhidraTypedefs();

		void loadBefore() const;

		void loadAfter() const;

		DataTypePtr getType(DB::Id id);

		DataTypePtr getDefaultType(int size, bool sign = false, bool floating = false);

		DataTypePtr calcDataTypeForNumber(uint64_t value);

		DataType::IFunctionSignature* getDefaultFuncSignature() const;

		DataType::IType* findTypeById(DB::Id id);

		DataType::IType* findTypeByName(const std::string& typeName);
	private:
		DB::DataTypeMapper* m_dataTypeMapper;
		DataType::FunctionSignature* m_defSignature;
		
	};
};