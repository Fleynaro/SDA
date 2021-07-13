#include "AddressSpaceMapper.h"
#include <managers/AddressSpaceManager.h>
#include <managers/ImageManager.h>

using namespace DB;
using namespace CE;

AddressSpaceMapper::AddressSpaceMapper(IRepository* repository)
	: AbstractMapper(repository)
{}

void AddressSpaceMapper::loadAll() {
	auto& db = getManager()->getProject()->getDB();
	Statement query(db, "SELECT * FROM sda_address_spaces");
	load(&db, query);
}

Id AddressSpaceMapper::getNextId() {
	auto& db = getManager()->getProject()->getDB();
	return GenerateNextId(&db, "sda_address_spaces");
}

AddressSpaceManager* AddressSpaceMapper::getManager() const
{
	return static_cast<AddressSpaceManager*>(m_repository);
}

IDomainObject* AddressSpaceMapper::doLoad(Database* db, Statement& query) {
	const int as_id = query.getColumn("as_id");
	const std::string name = query.getColumn("name");
	const std::string comment = query.getColumn("comment");

	auto addressSpace = getManager()->createAddressSpace(name, comment, false);

	addressSpace->setId(as_id);
	return addressSpace;
}

void AddressSpaceMapper::doInsert(TransactionContext* ctx, IDomainObject* obj) {
	doUpdate(ctx, obj);
}

void AddressSpaceMapper::doUpdate(TransactionContext* ctx, IDomainObject* obj) {
	auto as = dynamic_cast<AddressSpace*>(obj);
	Statement query(*ctx->m_db, "REPLACE INTO sda_address_spaces (as_id, name, comment, save_id) VALUES(?1, ?2, ?3, ?4)");
	query.bind(1, as->getId());
	bind(query, *as);
	query.bind(4, ctx->m_saveId);
	query.exec();
}

void AddressSpaceMapper::doRemove(TransactionContext* ctx, IDomainObject* obj) {
	const std::string action_query_text =
		ctx->m_notDelete ? "UPDATE sda_address_spaces SET deleted=1" : "DELETE FROM sda_address_spaces";
	Statement query(*ctx->m_db, action_query_text + " WHERE as_id=?1");
	query.bind(1, obj->getId());
	query.exec();
}

void AddressSpaceMapper::bind(Statement& query, AddressSpace& as) {
	query.bind(2, as.getName());
	query.bind(3, as.getComment());
}
