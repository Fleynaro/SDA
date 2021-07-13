#pragma once
#include "GhidraDataTypeMapper.h"
#include <datatypes/Enum.h>

namespace CE::Ghidra
{
	class EnumTypeMapper : public IMapper
	{
	public:
		EnumTypeMapper(DataTypeMapper* dataTypeMapper);

		void load(packet::SDataFullSyncPacket* dataPacket) override;

		void upsert(SyncContext* ctx, IObject* obj) override;

		void remove(SyncContext* ctx, IObject* obj) override;

	private:
		DataTypeMapper* m_dataTypeMapper;

		SDataTypeEnum buildDesc(DataType::Enum* Enum) const;

		void changeEnumByDesc(DataType::Enum* Enum, const SDataTypeEnum& enumDesc) const;
	};
};