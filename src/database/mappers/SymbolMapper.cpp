#include "SymbolMapper.h"
#include <symbols/Symbol.h>
#include <managers/TypeManager.h>
#include <managers/SymbolManager.h>

using namespace DB;
using namespace CE;
using namespace Symbol;

SymbolMapper::SymbolMapper(IRepository* repository)
	: AbstractMapper(repository)
{}

void SymbolMapper::loadAll() {
	auto& db = getManager()->getProject()->getDB();
	Statement query(db, "SELECT * FROM sda_symbols");
	load(&db, query);
}

Id SymbolMapper::getNextId() {
	auto& db = getManager()->getProject()->getDB();
	return GenerateNextId(&db, "sda_symbols");
}

SymbolManager* SymbolMapper::getManager() {
	return static_cast<SymbolManager*>(m_repository);
}

IDomainObject* SymbolMapper::doLoad(Database* db, SQLite::Statement& query) {
	int symbol_id = query.getColumn("symbol_id");
	auto type = (Symbol::Type)(int)query.getColumn("type");
	std::string name = query.getColumn("name");
	std::string comment = query.getColumn("comment");
	std::string json_extra_str = query.getColumn("json_extra");
	auto json_extra = json::parse(json_extra_str);

	DataType::IType* dataType;
	try {
		dataType = getManager()->getProject()->getTypeManager()->findTypeById(query.getColumn("type_id"));
	}
	catch (AbstractItemManager::ItemNotFoundException ex) {
		dataType = getManager()->getProject()->getTypeManager()->getFactory().getDefaultType();
	}
	auto dataTypeUnit = DataType::GetUnit(dataType, query.getColumn("pointer_lvl"));

	AbstractSymbol* symbol = nullptr;
	auto factory = getManager()->getFactory(false);
	switch (type)
	{
	case FUNCTION: {
		auto offset = json_extra["offset"].get<int64_t>();
		auto funcSig = dynamic_cast<DataType::IFunctionSignature*>(dataType);
		symbol = factory.createFunctionSymbol(offset, funcSig, name, comment);
		break;
	}
	case GLOBAL_VAR: {
		auto offset = json_extra["offset"].get<int64_t>();
		symbol = factory.createGlobalVarSymbol(offset, dataTypeUnit, name, comment);
		break;
	}
	case LOCAL_INSTR_VAR: {
		symbol = factory.createLocalInstrVarSymbol(dataTypeUnit, name, comment);
		break;
	}
	case LOCAL_STACK_VAR: {
		auto offset = json_extra["offset"].get<int64_t>();
		symbol = factory.createLocalStackVarSymbol(offset, dataTypeUnit, name, comment);
		break;
	}
	case FUNC_PARAMETER: {
		auto paramIdx = json_extra["param_idx"].get<int>();
		symbol = factory.createFuncParameterSymbol(paramIdx, nullptr, dataTypeUnit, name, comment);
		break;
	}
	case STRUCT_FIELD: {
		auto absBitOffset = json_extra["abs_bit_offset"].get<int>();
		auto bitSize = json_extra["bit_size"].get<int>();
		symbol = factory.createStructFieldSymbol(absBitOffset, bitSize, nullptr, dataTypeUnit, name, comment);
		break;
	}
	}

	symbol->setId(symbol_id);
	return symbol;
}

void SymbolMapper::doInsert(TransactionContext* ctx, IDomainObject* obj) {
	doUpdate(ctx, obj);
}

void SymbolMapper::doUpdate(TransactionContext* ctx, IDomainObject* obj) {
	auto symbol = dynamic_cast<AbstractSymbol*>(obj);
	SQLite::Statement query(*ctx->m_db, "REPLACE INTO sda_symbols (symbol_id, type, name, type_id, pointer_lvl, comment, json_extra, save_id, ghidra_sync_id) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, 0)");
	query.bind(1, symbol->getId());
	bind(query, *symbol);
	query.bind(8, ctx->m_saveId);
	query.exec();
}

void SymbolMapper::doRemove(TransactionContext* ctx, IDomainObject* obj) {
	std::string action_query_text =
		ctx->m_notDelete ? "UPDATE sda_symbols SET deleted=1" : "DELETE FROM sda_symbols";
	Statement query(*ctx->m_db, action_query_text + " WHERE symbol_id=?1");
	query.bind(1, obj->getId());
	query.exec();
}

void SymbolMapper::bind(SQLite::Statement& query, AbstractSymbol& symbol) {
	auto type = dynamic_cast<DB::IDomainObject*>(symbol.getDataType()->getType());
	query.bind(2, symbol.getType());
	query.bind(3, symbol.getName());
	query.bind(4, type->getId());
	query.bind(5, DataType::GetPointerLevelStr(symbol.getDataType()));
	query.bind(6, symbol.getComment());

	json json_extra;
	if (auto memSymbol = dynamic_cast<AbstractMemorySymbol*>(&symbol)) {
		json_extra["offset"] = memSymbol->getOffset();
	}
	else if (auto funcParamSymbol = dynamic_cast<FuncParameterSymbol*>(&symbol)) {
		json_extra["param_idx"] = funcParamSymbol->getParamIdx();
	} 
	else if(auto structFieldSymbol = dynamic_cast<StructFieldSymbol*>(&symbol)) {
		json_extra["abs_bit_offset"] = structFieldSymbol->getAbsBitOffset();
		json_extra["bit_size"] = structFieldSymbol->getBitSize();
	}
	query.bind(7, json_extra.dump());
}
