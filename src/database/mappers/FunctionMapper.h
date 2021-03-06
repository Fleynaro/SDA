#pragma once
#include <database/AbstractMapper.h>
#include <Function.h>

namespace CE {
	class FunctionManager;
};

namespace DB
{
	class FunctionMapper : public AbstractMapper
	{
	public:
		FunctionMapper(CE::FunctionManager* repository);

		void loadAll();

		Id getNextId() override;

		CE::FunctionManager* getManager() const;
	protected:
		IDomainObject* doLoad(Database* db, Statement& query) override;

		void doInsert(TransactionContext* ctx, IDomainObject* obj) override;

		void doUpdate(TransactionContext* ctx, IDomainObject* obj) override;

		void doRemove(TransactionContext* ctx, IDomainObject* obj) override;

	private:
		void bind(Statement& query, CE::Function* func);
	};
};