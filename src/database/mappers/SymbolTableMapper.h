#pragma once
#include <database/AbstractMapper.h>
#include <SymbolTable.h>

namespace CE {
	class SymbolTableManager;
};

namespace DB
{
	class SymbolTableMapper : public AbstractMapper
	{
	public:
		SymbolTableMapper(IRepository* repository);

		void loadAll();

		Id getNextId() override;

		CE::SymbolTableManager* getManager() const;
	protected:
		IDomainObject* doLoad(Database* db, Statement& query) override;

		void doInsert(TransactionContext* ctx, IDomainObject* obj) override;

		void doUpdate(TransactionContext* ctx, IDomainObject* obj) override;

		void doRemove(TransactionContext* ctx, IDomainObject* obj) override;

	private:
		void bind(Statement& query, CE::Symbol::AbstractSymbolTable* symbolTable);
	};
};