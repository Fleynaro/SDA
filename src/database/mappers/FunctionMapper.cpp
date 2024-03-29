#include "FunctionMapper.h"
#include <managers/FunctionManager.h>
#include <managers/SymbolTableManager.h>
#include <managers/SymbolManager.h>
#include <managers/ImageManager.h>

using namespace DB;
using namespace CE;

FunctionMapper::FunctionMapper(FunctionManager* repository)
	: AbstractMapper(repository)
{}

void FunctionMapper::loadAll() {
	auto& db = getManager()->getProject()->getDB();
	Statement query(db, "SELECT * FROM sda_functions WHERE deleted=0");
	load(&db, query);
}

Id FunctionMapper::getNextId() {
	auto& db = getManager()->getProject()->getDB();
	return GenerateNextId(&db, "sda_functions");
}

FunctionManager* FunctionMapper::getManager() const
{
	return static_cast<FunctionManager*>(m_repository);
}

IDomainObject* FunctionMapper::doLoad(Database* db, Statement& query) {
	const int func_id = query.getColumn("func_id");
	const int func_symbol_id = query.getColumn("func_symbol_id");
	const int stack_sym_table_id = query.getColumn("stack_sym_table_id");
	const int image_id = query.getColumn("image_id");

	auto funcSymbol = dynamic_cast<Symbol::FunctionSymbol*>(getManager()->getProject()->getSymbolManager()->findSymbolById(func_symbol_id));
	const auto stackSymTable = dynamic_cast<Symbol::StackSymbolTable*>(getManager()->getProject()->getSymTableManager()->findSymbolTableById(stack_sym_table_id));
	const auto image = getManager()->getProject()->getImageManager()->findImageById(image_id);

	auto function = getManager()->getFactory(false).createFunction(funcSymbol, image, stackSymTable);
	
	funcSymbol->setFunction(function);
	function->setId(func_id);
	return function;
}

void FunctionMapper::doInsert(TransactionContext* ctx, IDomainObject* obj) {
	doUpdate(ctx, obj);
}

void FunctionMapper::doUpdate(TransactionContext* ctx, IDomainObject* obj) {
	auto func = dynamic_cast<Function*>(obj);

	Statement query(*ctx->m_db, "REPLACE INTO sda_functions (func_id, func_symbol_id, stack_sym_table_id, image_id, save_id, ghidra_sync_id)\
				VALUES(?1, ?2, ?3, ?4, ?5, 0)");
	query.bind(1, func->getId());
	bind(query, func);
	query.bind(5, ctx->m_saveId);
	query.exec();
}

void FunctionMapper::doRemove(TransactionContext* ctx, IDomainObject* obj) {
	const std::string action_query_text =
		ctx->m_notDelete ? "UPDATE sda_functions SET deleted=1" : "DELETE FROM sda_functions";
	Statement query(*ctx->m_db, action_query_text + " WHERE func_id=?1");
	query.bind(1, obj->getId());
	query.exec();
}

void FunctionMapper::bind(Statement& query, Function* func) {
	query.bind(2, func->getFunctionSymbol()->getId());
	query.bind(3, func->getStackSymbolTable()->getId());
	query.bind(4, func->getImage()->getId());
}
