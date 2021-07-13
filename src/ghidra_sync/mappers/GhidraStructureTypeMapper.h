#pragma once
#include "GhidraDataTypeMapper.h"
#include <datatypes/Structure.h>

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

		SDataTypeStructure buildDesc(DataType::Structure* Struct) const;

		void changeStructureByDesc(DataType::Structure* Struct, const SDataTypeStructure& structDesc) const;
	};
};