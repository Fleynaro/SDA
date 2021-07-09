#include "Transaction.h"
#include <chrono>

using namespace SQLite;

DB::Transaction::Transaction(Database* db)
	: m_db(db)
{}

void DB::Transaction::markAsNew(IDomainObject* obj) {
	m_insertedObjs.remove(obj);
	m_insertedObjs.push_back(obj);

	obj->setId(obj->getMapper()->getNextId());

	if (obj->getMapper()->getRepository() != nullptr)
		obj->getMapper()->getRepository()->onChangeBeforeCommit(obj, IRepository::Inserted);
}

void DB::Transaction::markAsDirty(IDomainObject* obj) {
	m_updatedObjs.remove(obj);
	m_updatedObjs.push_back(obj);

	if (obj->getMapper()->getRepository() != nullptr)
		obj->getMapper()->getRepository()->onChangeBeforeCommit(obj, IRepository::Updated);
}

void DB::Transaction::markAsRemoved(IDomainObject* obj) {
	m_removedObjs.remove(obj);
	m_removedObjs.push_back(obj);

	if (obj->getMapper()->getRepository() != nullptr)
		obj->getMapper()->getRepository()->onChangeBeforeCommit(obj, IRepository::Removed);
}

void DB::Transaction::commit() {
	SQLite::Transaction transaction(*m_db);

	TransactionContext ctx;
	ctx.m_saveId = createSaveRecord();
	ctx.m_db = m_db;

	for (auto obj : m_insertedObjs) {
		if (obj->getMapper() != nullptr) {
			obj->getMapper()->insert(&ctx, obj);
			m_updatedObjs.remove(obj);
		}
	}

	for (auto obj : m_updatedObjs) {
		if (obj->getMapper() != nullptr)
			obj->getMapper()->update(&ctx, obj);
	}

	for (auto obj : m_removedObjs) {
		if (obj->getMapper() != nullptr) {
			obj->getMapper()->remove(&ctx, obj);
			delete obj;
		}
	}
	
	transaction.commit();
}

DB::Id DB::Transaction::createSaveRecord() const
{
	using namespace std::chrono;
	SQLite::Statement query(*m_db, "INSERT INTO sda_saves (date, insertsCount, updatesCount, deletesCount) VALUES(?1, ?2, ?3, ?4)");
	query.bind(1, duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
	query.bind(2, static_cast<int>(m_insertedObjs.size()));
	query.bind(3, static_cast<int>(m_updatedObjs.size()));
	query.bind(4, static_cast<int>(m_removedObjs.size()));
	query.exec();
	return static_cast<DB::Id>(m_db->getLastInsertRowid());
}
