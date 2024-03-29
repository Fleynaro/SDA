#include "DataTypeMapper.h"
#include "SymbolMapper.h"
#include <managers/TypeManager.h>
#include <managers/SymbolManager.h>

using namespace DB;
using namespace CE;
using namespace DataType;


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

		const auto userDefType = dynamic_cast<UserDefinedType*>(getManager()->findTypeById(type_id));
		loadExtraJson(userDefType, json_extra);
	}
}

Id DataTypeMapper::getNextId() {
	auto& db = getManager()->getProject()->getDB();
	return GenerateNextId(&db, "sda_types");
}

TypeManager* DataTypeMapper::getManager() const
{
	return static_cast<TypeManager*>(m_repository);
}

IDomainObject* DataTypeMapper::doLoad(Database* db, Statement& query) {
	const std::string name = query.getColumn("name");
	const std::string comment = query.getColumn("comment");
	const int group = query.getColumn("group");
	std::string json_extra_str = query.getColumn("json_extra");
	auto json_extra = json::parse(json_extra_str);

	IDomainObject* obj = nullptr;
	const auto factory = getManager()->getFactory(false);
	switch (group)
	{
	case AbstractType::Group::Typedef:
		obj = factory.createTypedef(name, comment);
		break;
	case AbstractType::Group::Enum:
		obj = factory.createEnum(name, comment);
		break;
	case AbstractType::Group::Structure:
		obj = factory.createStructure(name, comment);
		break;
	case AbstractType::Group::FunctionSignature:
		const auto calling_convention = json_extra["calling_convention"].get<CallingConvetion>();
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
	auto type = dynamic_cast<UserDefinedType*>(obj);
	Statement query(*ctx->m_db, "REPLACE INTO sda_types (type_id, `group`, name, comment, json_extra, save_id, ghidra_sync_id) VALUES(?1, ?2, ?3, ?4, ?5, ?6, 0)");
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

void DataTypeMapper::loadExtraJson(UserDefinedType* userDefType, json json_extra) {
	if (auto Typedef = dynamic_cast<DataType::Typedef*>(userDefType))
	{
		const auto refDataType = DeserializeDataType(json_extra["ref_type"], getManager());
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
			const auto fieldSymbolId = json_field_symbol["id"].get<Id>();
			const auto fieldOffset = json_field_symbol["offset"].get<int>();
			const auto fieldSymbol = dynamic_cast<Symbol::StructFieldSymbol*>(symbolManager->findSymbolById(fieldSymbolId));
			Structure->getFields().addField(fieldOffset, fieldSymbol);
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
			FunctionSignature->getCustomStorages().emplace_back(idx, storage);
		}

		// load parameters
		auto symbolManager = getManager()->getProject()->getSymbolManager();
		for (const auto& json_param_symbol : json_extra["param_symbols"]) {
			const auto paramSymbolId = json_param_symbol.get<Id>();
			const auto paramSymbol = dynamic_cast<Symbol::FuncParameterSymbol*>(symbolManager->findSymbolById(paramSymbolId));
			FunctionSignature->getParameters().addParameter(paramSymbol);
		}

		// load other
		const auto retDataType = DeserializeDataType(json_extra["ret_type"], getManager());
		FunctionSignature->setReturnType(retDataType);
	}
}

json DataTypeMapper::createExtraJson(UserDefinedType* userDefType) {
	json json_extra;

	if (const auto Typedef = dynamic_cast<DataType::Typedef*>(userDefType))
	{
		json_extra["ref_type"] = SerializeDataType(Typedef->getRefType());
	}
	else if (auto Enum = dynamic_cast<DataType::Enum*>(userDefType))
	{
		json json_fields;
		for (const auto& [value, name] : Enum->getFields()) {
			json json_field;
			json_field["name"] = name;
			json_field["value"] = value;
			json_fields.push_back(json_field);
		}
		json_extra["fields"] = json_fields;
	}
	else if (auto Structure = dynamic_cast<DataType::Structure*>(userDefType))
	{
		// save fields
		json json_fields;
		for (const auto& [offset, fieldSymbol] : Structure->getFields()) {
			json json_field;
			json_field["id"] = fieldSymbol->getId();
			json_field["offset"] = fieldSymbol->getAbsBitOffset();
			json_fields.push_back(json_field);
		}
		json_extra["field_symbols"] = json_fields;

		// save other
		json_extra["size"] = Structure->getSize();
	}
	else if (auto FuncSignature = dynamic_cast<FunctionSignature*>(userDefType))
	{
		// save storages
		json json_storages;
		for (const auto& [idx, storage] : FuncSignature->getCustomStorages()) {
			json json_storage;
			json_storage["idx"] = idx;
			json_storage["type"] = storage.getType();
			json_storage["reg_id"] = storage.getRegGenericId();
			json_storage["offset"] = storage.getOffset();
			json_storages.push_back(json_storage);
		}
		json_extra["storages"] = json_storages;

		// save parameters
		json json_params;
		for (int i = 0; i < FuncSignature->getParameters().getParamsCount(); i++) {
			const auto param = FuncSignature->getParameters()[i];
			json_params.push_back(param->getId());
		}
		json_extra["param_symbols"] = json_params;

		// save other
		json_extra["calling_convention"] = FuncSignature->getCallingConvetion();
		json_extra["ret_type"] = SerializeDataType(FuncSignature->getReturnType());
	}

	return json_extra;
}

void DataTypeMapper::bind(Statement& query, UserDefinedType* userDefType)
{
	const auto json_extra = createExtraJson(userDefType);
	query.bind(2, userDefType->getGroup());
	query.bind(3, userDefType->getName());
	query.bind(4, userDefType->getComment());
	query.bind(5, json_extra.dump());
}

CE::DataTypePtr DB::DeserializeDataType(json json_dataType, CE::TypeManager* manager) {
	const auto id = json_dataType["id"].get<Id>();
	const auto ptr_lvl = json_dataType["ptr_lvl"].get<std::string>();
	const auto type = manager->findTypeById(id);
	return CE::DataType::GetUnit(type, ptr_lvl);
}

json DB::SerializeDataType(CE::DataTypePtr dataType) {
	auto refType = dynamic_cast<IDomainObject*>(dataType->getType());
	json json_dataType;
	json_dataType["id"] = refType->getId();
	json_dataType["ptr_lvl"] = GetPointerLevelStr(dataType);
	return json_dataType;
}
