#include "SymbolTableMapper.h"
#include "ImageDecorator.h"
#include <managers/SymbolTableManager.h>
#include <managers/SymbolManager.h>

using namespace DB;
using namespace CE;
using namespace Symbol;

SymbolTableMapper::SymbolTableMapper(IRepository* repository)
	: AbstractMapper(repository)
{}

void SymbolTableMapper::loadAll() {
	auto& db = getManager()->getProject()->getDB();
	Statement query(db, "SELECT * FROM sda_symbol_tables");
	load(&db, query);
}

Id SymbolTableMapper::getNextId() {
	auto& db = getManager()->getProject()->getDB();
	return GenerateNextId(&db, "sda_symbol_tables");
}

SymbolTableManager* SymbolTableMapper::getManager() const
{
	return static_cast<SymbolTableManager*>(m_repository);
}

IDomainObject* SymbolTableMapper::doLoad(Database* db, Statement& query) {
	const int sym_table_id = query.getColumn("sym_table_id");
	const auto type = static_cast<AbstractSymbolTable::SymbolTableType>(static_cast<int>(query.getColumn("type")));
	std::string json_symbols_str = query.getColumn("json_symbols");
	auto json_symbols = json::parse(json_symbols_str);

	// create a symbol table
	AbstractSymbolTable* symbolTable = nullptr;
	switch(type) {
	case AbstractSymbolTable::GLOBAL_SPACE:
		symbolTable = getManager()->getFactory(false).createGlobalSymbolTable();
		break;
	case AbstractSymbolTable::STACK_SPACE:
		symbolTable = getManager()->getFactory(false).createStackSymbolTable();
		break;
	}

	// load symbols for the symbol table
	for (const auto& json_symbol : json_symbols) {
		const auto symbol_id = json_symbol["sym_id"].get<Id>();
		const auto offset = json_symbol["offset"].get<int64_t>();

		const auto symbol = getManager()->getProject()->getSymbolManager()->findSymbolById(symbol_id);
		symbolTable->addSymbol(symbol, offset);
	}

	symbolTable->setId(sym_table_id);
	return symbolTable;
}

void SymbolTableMapper::doInsert(TransactionContext* ctx, IDomainObject* obj) {
	doUpdate(ctx, obj);
}

void SymbolTableMapper::doUpdate(TransactionContext* ctx, IDomainObject* obj) {
	auto symTable = dynamic_cast<AbstractSymbolTable*>(obj);
	Statement query(*ctx->m_db, "REPLACE INTO sda_symbol_tables (sym_table_id, type, json_symbols, save_id) VALUES(?1, ?2, ?3, ?4)");
	query.bind(1, symTable->getId());
	bind(query, symTable);
	query.bind(4, ctx->m_saveId);
	query.exec();
}

void SymbolTableMapper::doRemove(TransactionContext* ctx, IDomainObject* obj) {
	const std::string action_query_text =
		ctx->m_notDelete ? "UPDATE sda_symbol_tables SET deleted=1" : "DELETE FROM sda_symbol_tables";
	Statement query(*ctx->m_db, action_query_text + " WHERE sym_table_id=?1");
	query.bind(1, obj->getId());
	query.exec();
}

void SymbolTableMapper::bind(Statement& query, AbstractSymbolTable* symbolTable) {
	json json_symbols;
	for (const auto& [offset, symbol] : symbolTable->getSymbols()) {
		json json_symbol;
		json_symbol["sym_id"] = symbol->getId();
		json_symbol["offset"] = offset;
		json_symbols.push_back(json_symbol);
	}
	
	query.bind(2, symbolTable->getType());
	query.bind(3, json_symbols.dump());
}
