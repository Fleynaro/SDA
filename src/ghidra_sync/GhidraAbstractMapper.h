#pragma once
#include "GhidraObject.h"
#include <ghidra_sync/DataSyncPacketManagerService.h>

namespace SQLite {
	class Database;
};

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