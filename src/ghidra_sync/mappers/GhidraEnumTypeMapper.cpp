#include "GhidraEnumTypeMapper.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace Ghidra;

EnumTypeMapper::EnumTypeMapper(DataTypeMapper* dataTypeMapper)
	: m_dataTypeMapper(dataTypeMapper)
{}

void EnumTypeMapper::load(packet::SDataFullSyncPacket* dataPacket) {
	for (auto enumDesc : dataPacket->enums) {
		const auto type = m_dataTypeMapper->m_typeManager->findTypeByGhidraId(enumDesc.type.id);
		if (type == nullptr)
			throw std::exception("item not found");
		if (const auto Enum = dynamic_cast<DataType::Enum*>(type)) {
			changeEnumByDesc(Enum, enumDesc);
		}
	}
}

void EnumTypeMapper::upsert(SyncContext* ctx, IObject* obj) {
	const auto type = dynamic_cast<DataType::Enum*>(obj);
	ctx->m_dataPacket->enums.push_back(buildDesc(type));
	m_dataTypeMapper->upsert(ctx, obj);
}

void EnumTypeMapper::remove(SyncContext* ctx, IObject* obj) {
	m_dataTypeMapper->remove(ctx, obj);
}

SDataTypeEnum EnumTypeMapper::buildDesc(DataType::Enum* Enum) const
{
	SDataTypeEnum enumDesc;
	enumDesc.__set_type(m_dataTypeMapper->buildDesc(Enum));
	for (auto& field : Enum->getFields()) {
		SDataTypeEnumField enumFieldDesc;
		enumFieldDesc.__set_name(field.second);
		enumFieldDesc.__set_value(field.first);
		enumDesc.fields.push_back(enumFieldDesc);
	}
	return enumDesc;
}

void EnumTypeMapper::changeEnumByDesc(DataType::Enum* Enum, const SDataTypeEnum& enumDesc) const
{
	m_dataTypeMapper->changeUserTypeByDesc(Enum, enumDesc.type);
	Enum->setSize(enumDesc.type.size);
	Enum->deleteAll();
	for (auto& field : enumDesc.fields) {
		Enum->addField(field.name, field.value);
	}
}
