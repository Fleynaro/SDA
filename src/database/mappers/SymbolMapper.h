#pragma once
#include <database/AbstractMapper.h>
#include <symbols/AbstractSymbol.h>

namespace CE {
	class SymbolManager;
};

namespace DB
{
	class SymbolMapper : public AbstractMapper
	{
	public:
		SymbolMapper(IRepository* repository);

		void loadAll();

		Id getNextId() override;

		CE::SymbolManager* getManager() const;
	protected:
		IDomainObject* doLoad(Database* db, Statement& query) override;

		void doInsert(TransactionContext* ctx, IDomainObject* obj) override;

		void doUpdate(TransactionContext* ctx, IDomainObject* obj) override;

		void doRemove(TransactionContext* ctx, IDomainObject* obj) override;

	private:
		void bind(Statement& query, CE::Symbol::AbstractSymbol& symbol);
	};

	json SerializeSymbol(CE::Symbol::ISymbol* symbol);

	void DeserializeSymbol(CE::Symbol::ISymbol* symbol, json symbol_json);
};