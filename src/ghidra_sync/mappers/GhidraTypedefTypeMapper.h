#pragma once
#include "GhidraDataTypeMapper.h"
#include <datatypes/Typedef.h>

namespace CE::Ghidra
{
	class TypedefTypeMapper : public IMapper
	{
	public:
		TypedefTypeMapper(DataTypeMapper* dataTypeMapper);

		void load(packet::SDataFullSyncPacket* dataPacket) override;

		void upsert(SyncContext* ctx, IObject* obj) override;

		void remove(SyncContext* ctx, IObject* obj) override;

	private:
		DataTypeMapper* m_dataTypeMapper;

		datatype::SDataTypeTypedef buildDesc(DataType::Typedef* Typedef);

		void changeTypedefByDesc(DataType::Typedef* Typedef, const datatype::SDataTypeTypedef& typedefDesc);
	};
};