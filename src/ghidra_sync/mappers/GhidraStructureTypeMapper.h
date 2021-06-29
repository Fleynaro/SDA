#pragma once
#include "GhidraDataTypeMapper.h"
#include <Code/Type/Structure.h>

namespace CE::Ghidra
{
	class StructureTypeMapper : public IMapper
	{
	public:
		DataTypeMapper* m_dataTypeMapper;

		StructureTypeMapper(DataTypeMapper* dataTypeMapper);

		void load(packet::SDataFullSyncPacket* dataPacket) override;

		void upsert(SyncContext* ctx, IObject* obj) override;

		void remove(SyncContext* ctx, IObject* obj) override;

		datatype::SDataTypeStructure buildDesc(DataType::Structure* Struct);

		void changeStructureByDesc(DataType::Structure* Struct, const datatype::SDataTypeStructure& structDesc);
	};
};