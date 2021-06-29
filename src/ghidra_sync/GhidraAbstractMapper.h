#pragma once
#include "GhidraObject.h"
#include "DataSyncPacketManagerService.h"
#include <SQLiteCpp/SQLiteCpp.h>

namespace CE::Ghidra
{
	using namespace ghidra;

	struct SyncContext
	{
		int m_syncId;
		packet::SDataFullSyncPacket* m_dataPacket;
		SQLite::Database* m_db;
	};

	class IMapper
	{
	public:
		virtual void load(packet::SDataFullSyncPacket* dataPacket) = 0;
		virtual void upsert(SyncContext* ctx, IObject* obj) = 0;
		virtual void remove(SyncContext* ctx, IObject* obj) = 0;
	};
};