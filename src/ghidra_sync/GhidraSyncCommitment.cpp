#include "GhidraSyncCommitment.h"
#include <Project.h>

using namespace CE;
using namespace Ghidra;

SyncCommitment::SyncCommitment(Sync* sync)
	: m_sync(sync)
{}

void SyncCommitment::upsert(IObject* obj) {
	m_upsertedObjs.push_back(obj);
}

void SyncCommitment::remove(IObject* obj) {
	m_removedObjs.push_back(obj);
}

void SyncCommitment::commit() {
	packet::SDataFullSyncPacket dataPacket;
	SyncContext ctx;
	auto& db = m_sync->getProject()->getDB();
	Transaction transaction(db);

	ctx.m_syncId = createSyncRecord();
	ctx.m_dataPacket = &dataPacket;
	ctx.m_db = &db;

	for (auto obj : m_upsertedObjs) {
		obj->getGhidraMapper()->upsert(&ctx, obj);
	}

	for (auto obj : m_removedObjs) {
		obj->getGhidraMapper()->remove(&ctx, obj);
	}

	transaction.commit();
	
	Transport tr(m_sync->getClient());
	m_sync->getDataSyncPacketManagerServiceClient()->sendFullSyncPacket(dataPacket);
}

int SyncCommitment::createSyncRecord() const
{
	using namespace std::chrono;
	auto& db = m_sync->getProject()->getDB();
	Statement query(db, "INSERT INTO sda_ghidra_sync (date, type, comment, objectsCount) VALUES(?1, ?2, ?3, ?4)");
	query.bind(1, duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
	query.bind(2, 1);
	query.bind(3, "");
	query.bind(4, static_cast<int>(m_upsertedObjs.size()) + static_cast<int>(m_removedObjs.size()));
	query.exec();
	return static_cast<int>(db.getLastInsertRowid());
}
