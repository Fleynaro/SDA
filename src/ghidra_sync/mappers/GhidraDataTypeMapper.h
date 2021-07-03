#pragma once
#include <ghidra_sync/GhidraAbstractMapper.h>
#include <datatypes/UserType.h>

namespace CE {
	class TypeManager;
};

namespace CE::Ghidra
{
	using namespace ghidra;
	using namespace ghidra::datatype;

	class EnumTypeMapper;
	class StructureTypeMapper;
	class ClassTypeMapper;
	class TypedefTypeMapper;
	class SignatureTypeMapper;

	class DataTypeMapper : public IMapper
	{
	public:
		EnumTypeMapper* m_enumTypeMapper;
		StructureTypeMapper* m_structureTypeMapper;
		ClassTypeMapper* m_classTypeMapper;
		TypedefTypeMapper* m_typedefTypeMapper;
		SignatureTypeMapper* m_signatureTypeMapper;
		CE::TypeManager* m_typeManager;

		DataTypeMapper(CE::TypeManager* typeManager);

		void load(packet::SDataFullSyncPacket* dataPacket) override;

		void upsert(SyncContext* ctx, IObject* obj) override;

		void remove(SyncContext* ctx, IObject* obj) override;

		datatype::SDataType buildDesc(DataType::UserDefinedType* type);

		shared::STypeUnit buildTypeUnitDesc(DataTypePtr type) const;

		DataTypePtr getTypeByDesc(const shared::STypeUnit& typeUnitDesc) const;

		void changeUserTypeByDesc(DataType::UserDefinedType* type, const datatype::SDataType& typeDesc);

	private:
		void createTypeByDescIfNotExists(const datatype::SDataType& typeDesc);

		DataType::UserDefinedType* createTypeByDesc(const datatype::SDataType& typeDesc);
	};
};