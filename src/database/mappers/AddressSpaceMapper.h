#pragma once
#include <DB/AbstractMapper.h>
#include <Address/AddressSpace.h>

namespace CE {
	class AddressSpaceManager;
};

namespace DB
{
	class AddressSpaceMapper : public AbstractMapper
	{
	public:
		AddressSpaceMapper(IRepository* repository);

		void loadAll();

		Id getNextId() override;

		CE::AddressSpaceManager* getManager();
	protected:
		IDomainObject* doLoad(Database* db, SQLite::Statement& query) override;

		void doInsert(TransactionContext* ctx, IDomainObject* obj) override;

		void doUpdate(TransactionContext* ctx, IDomainObject* obj) override;

		void doRemove(TransactionContext* ctx, IDomainObject* obj) override;

	private:
		void bind(SQLite::Statement& query, CE::AddressSpace& as);
	};
};