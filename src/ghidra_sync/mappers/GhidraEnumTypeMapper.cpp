#include "GhidraEnumTypeMapper.h"
#include <Manager/TypeManager.h>

using namespace CE;
using namespace CE::Ghidra;

EnumTypeMapper::EnumTypeMapper(DataTypeMapper* dataTypeMapper)
	: m_dataTypeMapper(dataTypeMapper)
{}

void EnumTypeMapper::load(packet::SDataFullSyncPacket* dataPacket) {
	for (auto enumDesc : dataPacket->enums) {
		auto type = m_dataTypeMapper->m_typeManager->findTypeByGhidraId(enumDesc.type.id);
		if (type == nullptr)
			throw std::exception("item not found");
		if (auto Enum = dynamic_cast<DataType::Enum*>(type)) {
			changeEnumByDesc(Enum, enumDesc);
		}
	}
}

void EnumTypeMapper::upsert(SyncContext* ctx, IObject* obj) {
	auto type = dynamic_cast<DataType::Enum*>(obj);
	ctx->m_dataPacket->enums.push_back(buildDesc(type));
	m_dataTypeMapper->upsert(ctx, obj);
}

void EnumTypeMapper::remove(SyncContext* ctx, IObject* obj) {
	m_dataTypeMapper->remove(ctx, obj);
}

datatype::SDataTypeEnum EnumTypeMapper::buildDesc(DataType::Enum* Enum) {
	datatype::SDataTypeEnum enumDesc;
	enumDesc.__set_type(m_dataTypeMapper->buildDesc(Enum));
	for (auto& field : Enum->getFields()) {
		datatype::SDataTypeEnumField enumFieldDesc;
		enumFieldDesc.__set_name(field.second);
		enumFieldDesc.__set_value(field.first);
		enumDesc.fields.push_back(enumFieldDesc);
	}
	return enumDesc;
}

void EnumTypeMapper::changeEnumByDesc(DataType::Enum* Enum, const datatype::SDataTypeEnum& enumDesc) {
	m_dataTypeMapper->changeUserTypeByDesc(Enum, enumDesc.type);
	Enum->setSize(enumDesc.type.size);
	Enum->deleteAll();
	for (auto& field : enumDesc.fields) {
		Enum->addField(field.name, field.value);
	}
}
