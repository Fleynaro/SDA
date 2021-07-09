#include "DataTypeMapper.h"
#include <managers/TypeManager.h>
#include <managers/SymbolManager.h>

using namespace DB;
using namespace CE;
using namespace CE::DataType;


DataTypeMapper::DataTypeMapper(IRepository* repository)
	: AbstractMapper(repository)
{}

void DataTypeMapper::loadBefore() {
	auto& db = getManager()->getProject()->getDB();
	Statement query(db, "SELECT * FROM sda_types WHERE type_id >= 1000 AND deleted = 0");
	load(&db, query);
}

void DataTypeMapper::loadAfter() {
	auto& db = getManager()->getProject()->getDB();
	Statement query(db, "SELECT type_id, json_extra FROM sda_types WHERE type_id >= 1000 AND deleted = 0");
	while (query.executeStep())
	{
		const int type_id = query.getColumn("type_id");
		std::string json_extra_str = query.getColumn("json_extra");
		const auto json_extra = json::parse(json_extra_str);

		const auto userDefType = dynamic_cast<DataType::UserDefinedType*>(getManager()->findTypeById(type_id));
		loadExtraJson(userDefType, json_extra);
	}
}

Id DataTypeMapper::getNextId() {
	auto& db = getManager()->getProject()->getDB();
	return GenerateNextId(&db, "sda_types");
}

CE::TypeManager* DataTypeMapper::getManager() const
{
	return static_cast<CE::TypeManager*>(m_repository);
}

IDomainObject* DataTypeMapper::doLoad(Database* db, SQLite::Statement& query) {
	const std::string name = query.getColumn("name");
	const std::string comment = query.getColumn("comment");
	const int group = query.getColumn("group");
	std::string json_extra_str = query.getColumn("json_extra");
	auto json_extra = json::parse(json_extra_str);

	IDomainObject* obj = nullptr;
	const auto factory = getManager()->getFactory(false);
	switch (group)
	{
	case DataType::AbstractType::Group::Typedef:
		obj = factory.createTypedef(name, comment);
		break;
	case DataType::AbstractType::Group::Enum:
		obj = factory.createEnum(name, comment);
		break;
	case DataType::AbstractType::Group::Structure:
		obj = factory.createStructure(name, comment);
		break;
	case DataType::AbstractType::Group::FunctionSignature:
		const auto calling_convention = json_extra["calling_convention"].get<DataType::FunctionSignature::CallingConvetion>();
		obj = factory.createSignature(calling_convention, name, comment);
		break;
	}

	if (obj != nullptr)
		obj->setId(query.getColumn("type_id"));
	return obj;
}

void DataTypeMapper::doInsert(TransactionContext* ctx, IDomainObject* obj) {
	doUpdate(ctx, obj);
}

void DataTypeMapper::doUpdate(TransactionContext* ctx, IDomainObject* obj) {
	auto type = dynamic_cast<CE::DataType::UserDefinedType*>(obj);
	SQLite::Statement query(*ctx->m_db, "REPLACE INTO sda_types (type_id, `group`, name, comment, json_extra, save_id, ghidra_sync_id) VALUES(?1, ?2, ?3, ?4, ?5, ?6, 0)");
	query.bind(1, type->getId());
	bind(query, type);
	query.bind(6, ctx->m_saveId);
	query.exec();
}

void DataTypeMapper::doRemove(TransactionContext* ctx, IDomainObject* obj) {
	const std::string action_query_text =
		ctx->m_notDelete ? "UPDATE sda_types SET deleted=1" : "DELETE FROM sda_types";
	Statement query(*ctx->m_db, action_query_text + " WHERE type_id=?1");
	query.bind(1, obj->getId());
	query.exec();
}

DataTypePtr DB::DataTypeMapper::loadDataTypeJson(json json_dataType) {
	const auto id = json_dataType["id"].get<DB::Id>();
	const auto ptr_lvl = json_dataType["ptr_lvl"].get<std::string>();
	const auto type = getManager()->findTypeById(id);
	return DataType::GetUnit(type, ptr_lvl);
}

json DB::DataTypeMapper::createDataTypeJson(DataTypePtr dataType) {
	auto refType = dynamic_cast<DB::IDomainObject*>(dataType->getType());
	json json_dataType;
	json_dataType["id"] = refType->getId();
	json_dataType["ptr_lvl"] = DataType::GetPointerLevelStr(dataType);
	return json_dataType;
}

void DB::DataTypeMapper::loadExtraJson(CE::DataType::UserDefinedType* userDefType, json json_extra) {
	if (auto Typedef = dynamic_cast<DataType::Typedef*>(userDefType))
	{
		const auto refDataType = loadDataTypeJson(json_extra["ref_type"]);
		Typedef->setRefType(refDataType);
	}
	else if (auto Enum = dynamic_cast<DataType::Enum*>(userDefType))
	{
		for (const auto& json_field : json_extra["fields"]) {
			const auto name = json_field["name"].get<std::string>();
			const auto value = json_field["value"].get<int>();
			Enum->addField(name, value);
		}
	}
	else if (auto Structure = dynamic_cast<DataType::Structure*>(userDefType))
	{
		// load fields
		auto symbolManager = getManager()->getProject()->getSymbolManager();
		for (const auto& json_field_symbol : json_extra["field_symbols"]) {
			const auto fieldSymbolId = json_field_symbol.get<DB::Id>();
			const auto fieldSymbol = dynamic_cast<CE::Symbol::StructFieldSymbol*>(symbolManager->findSymbolById(fieldSymbolId));
			Structure->addField(fieldSymbol);
		}

		// load other
		const auto size = json_extra["size"].get<int>();
		Structure->resize(size);
	}
	else if (auto FunctionSignature = dynamic_cast<DataType::FunctionSignature*>(userDefType))
	{
		// load storages
		for (const auto& json_storage : json_extra["storages"]) {
			auto idx = json_storage["idx"].get<int>();
			const auto storage_type = json_storage["type"].get<Decompiler::Storage::StorageType>();
			const auto register_id = json_storage["reg_id"].get<int>();
			const auto offset = json_storage["offset"].get<int64_t>();
			auto storage = Decompiler::Storage(storage_type, register_id, offset);
			FunctionSignature->getCustomStorages().push_back(std::pair(idx, storage));
		}

		// load parameters
		auto symbolManager = getManager()->getProject()->getSymbolManager();
		for (const auto& json_param_symbol : json_extra["param_symbols"]) {
			const auto paramSymbolId = json_param_symbol.get<DB::Id>();
			const auto paramSymbol = dynamic_cast<CE::Symbol::FuncParameterSymbol*>(symbolManager->findSymbolById(paramSymbolId));
			FunctionSignature->addParameter(paramSymbol);
		}

		// load other
		const auto retDataType = loadDataTypeJson(json_extra["ret_type"]);
		FunctionSignature->setReturnType(retDataType);
	}
}

json DB::DataTypeMapper::createExtraJson(CE::DataType::UserDefinedType* userDefType) {
	json json_extra;

	if (const auto Typedef = dynamic_cast<DataType::Typedef*>(userDefType))
	{
		json_extra["ref_type"] = createDataTypeJson(Typedef->getRefType());
	}
	else if (auto Enum = dynamic_cast<DataType::Enum*>(userDefType))
	{
		json json_fields;
		for (const auto& pair : Enum->getFields()) {
			json json_field;
			json_field["name"] = pair.second;
			json_field["value"] = pair.first;
			json_fields.push_back(json_field);
		}
		json_extra["fields"] = json_fields;
	}
	else if (auto Structure = dynamic_cast<DataType::Structure*>(userDefType))
	{
		// save fields
		json json_fields;
		for (const auto& pair : Structure->getFields()) {
			auto fieldSymbol = pair.second;
			json_fields.push_back(fieldSymbol->getId());
		}
		json_extra["field_symbols"] = json_fields;

		// save other
		json_extra["size"] = Structure->getSize();
	}
	else if (auto FuncSignature = dynamic_cast<DataType::FunctionSignature*>(userDefType))
	{
		// save storages
		json json_storages;
		for (auto& pair : FuncSignature->getCustomStorages()) {
			json json_storage;
			auto& storage = pair.second;
			json_storage["idx"] = pair.first;
			json_storage["type"] = storage.getType();
			json_storage["reg_id"] = storage.getRegisterId();
			json_storage["offset"] = storage.getOffset();
			json_storages.push_back(json_storage);
		}
		json_extra["storages"] = json_storages;

		// save parameters
		json json_params;
		for (auto paramSymbol : FuncSignature->getParameters()) {
			json_params.push_back(paramSymbol->getId());
		}
		json_extra["param_symbols"] = json_params;

		// save other
		json_extra["calling_convention"] = FuncSignature->getCallingConvetion();
		json_extra["ret_type"] = createDataTypeJson(FuncSignature->getReturnType());
	}

	return json_extra;
}

void DataTypeMapper::bind(SQLite::Statement& query, CE::DataType::UserDefinedType* userDefType)
{
	const auto json_extra = createExtraJson(userDefType);
	query.bind(2, userDefType->getGroup());
	query.bind(3, userDefType->getName());
	query.bind(4, userDefType->getComment());
	query.bind(5, json_extra.dump());
}
