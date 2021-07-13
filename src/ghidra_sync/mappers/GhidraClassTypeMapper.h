#pragma once
#include "GhidraStructureTypeMapper.h"
#include <datatypes/Class.h>

namespace CE::Ghidra
{
	class ClassTypeMapper : public IMapper
	{
	public:
		ClassTypeMapper(StructureTypeMapper* structTypeMapper);

		void load(packet::SDataFullSyncPacket* dataPacket) override;

		void upsert(SyncContext* ctx, IObject* obj) override;

		void remove(SyncContext* ctx, IObject* obj) override;

	private:
		StructureTypeMapper* m_structTypeMapper;

		SDataTypeClass buildDesc(DataType::Class* Class) const;

		void changeClassByDesc(DataType::Class* Class, const SDataTypeClass& classDesc) const;
	};
};