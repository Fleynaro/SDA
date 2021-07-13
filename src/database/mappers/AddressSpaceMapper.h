#pragma once
#include <database/AbstractMapper.h>
#include <AddressSpace.h>

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

		CE::AddressSpaceManager* getManager() const;
	protected:
		IDomainObject* doLoad(Database* db, Statement& query) override;

		void doInsert(TransactionContext* ctx, IDomainObject* obj) override;

		void doUpdate(TransactionContext* ctx, IDomainObject* obj) override;

		void doRemove(TransactionContext* ctx, IDomainObject* obj) override;

	private:
		void bind(Statement& query, CE::AddressSpace& as);
	};
};