#pragma once
#include <DB/AbstractMapper.h>
#include <Code/Symbol/SymbolTable/SymbolTable.h>

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

		CE::SymbolTableManager* getManager();
	protected:
		IDomainObject* doLoad(Database* db, SQLite::Statement& query) override;

		void doInsert(TransactionContext* ctx, IDomainObject* obj) override;

		void doUpdate(TransactionContext* ctx, IDomainObject* obj) override;

		void doRemove(TransactionContext* ctx, IDomainObject* obj) override;

	private:
		void bind(SQLite::Statement& query, CE::Symbol::SymbolTable* memoryArea);
	};
};