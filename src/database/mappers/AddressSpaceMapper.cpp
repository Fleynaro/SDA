#include "AddressSpaceMapper.h"
#include <managers/AddressSpaceManager.h>
#include <managers/ImageManager.h>

using namespace DB;
using namespace CE;

DB::AddressSpaceMapper::AddressSpaceMapper(IRepository* repository)
	: AbstractMapper(repository)
{}

void DB::AddressSpaceMapper::loadAll() {
	auto& db = getManager()->getProject()->getDB();
	Statement query(db, "SELECT * FROM sda_address_spaces");
	load(&db, query);
}

Id DB::AddressSpaceMapper::getNextId() {
	auto& db = getManager()->getProject()->getDB();
	return GenerateNextId(&db, "sda_address_spaces");
}

CE::AddressSpaceManager* DB::AddressSpaceMapper::getManager() const
{
	return static_cast<AddressSpaceManager*>(m_repository);
}

IDomainObject* DB::AddressSpaceMapper::doLoad(Database* db, SQLite::Statement& query) {
	int as_id = query.getColumn("as_id");
	std::string name = query.getColumn("name");
	std::string comment = query.getColumn("comment");

	auto addressSpace = getManager()->createAddressSpace(name, comment, false);

	addressSpace->setId(as_id);
	return addressSpace;
}

void DB::AddressSpaceMapper::doInsert(TransactionContext* ctx, IDomainObject* obj) {
	doUpdate(ctx, obj);
}

void DB::AddressSpaceMapper::doUpdate(TransactionContext* ctx, IDomainObject* obj) {
	auto as = dynamic_cast<AddressSpace*>(obj);
	SQLite::Statement query(*ctx->m_db, "REPLACE INTO sda_address_spaces (as_id, name, comment, save_id) VALUES(?1, ?2, ?3, ?4)");
	query.bind(1, as->getId());
	bind(query, *as);
	query.bind(4, ctx->m_saveId);
	query.exec();
}

void DB::AddressSpaceMapper::doRemove(TransactionContext* ctx, IDomainObject* obj) {
	std::string action_query_text =
		ctx->m_notDelete ? "UPDATE sda_address_spaces SET deleted=1" : "DELETE FROM sda_address_spaces";
	Statement query(*ctx->m_db, action_query_text + " WHERE as_id=?1");
	query.bind(1, obj->getId());
	query.exec();
}

void DB::AddressSpaceMapper::bind(SQLite::Statement& query, CE::AddressSpace& as) {
	query.bind(2, as.getName());
	query.bind(3, as.getComment());
}
