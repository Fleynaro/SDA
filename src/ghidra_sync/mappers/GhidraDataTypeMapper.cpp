#include "GhidraDataTypeMapper.h"
#include "GhidraEnumTypeMapper.h"
#include "GhidraStructureTypeMapper.h"
#include "GhidraClassTypeMapper.h"
#include "GhidraTypedefTypeMapper.h"
#include "GhidraSignatureTypeMapper.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace CE::Ghidra;

DataTypeMapper::DataTypeMapper(CE::TypeManager* typeManager)
	: m_typeManager(typeManager)
{
	m_enumTypeMapper = new EnumTypeMapper(this);
	m_structureTypeMapper = new StructureTypeMapper(this);
	m_classTypeMapper = new ClassTypeMapper(m_structureTypeMapper);
	m_typedefTypeMapper = new TypedefTypeMapper(this);
	m_signatureTypeMapper = new SignatureTypeMapper(this);
}

void DataTypeMapper::createTypeByDescIfNotExists(const datatype::SDataType& typeDesc)
{
	auto type = m_typeManager->findTypeByGhidraId(typeDesc.id);
	if (type == nullptr) {
		createTypeByDesc(typeDesc);
	}
}

DataType::UserDefinedType* DataTypeMapper::createTypeByDesc(const datatype::SDataType& typeDesc)
{
	DataType::UserDefinedType* userType = nullptr;
	/*switch (typeDesc.group)
	{
	case DataTypeGroup::Typedef:
		userType = m_typeManager->createTypedef(typeDesc.name, typeDesc.comment);
		break;
	case DataTypeGroup::Enum:
		userType = m_typeManager->createEnum(typeDesc.name, typeDesc.comment);
		break;
	case DataTypeGroup::Structure:
		userType = m_typeManager->createStructure(typeDesc.name, typeDesc.comment);
		break;
	case DataTypeGroup::Class:
		userType = m_typeManager->createClass(typeDesc.name, typeDesc.comment);
		break;
	case DataTypeGroup::Signature:
		userType = m_typeManager->createSignature(typeDesc.name, typeDesc.comment);
		break;
	}*/
	return userType;
}

void DataTypeMapper::load(packet::SDataFullSyncPacket* dataPacket) {
	for (auto it : dataPacket->typedefs) {
		createTypeByDescIfNotExists(it.type);
	}
	
	for (auto it : dataPacket->enums) {
		createTypeByDescIfNotExists(it.type);
	}

	for (auto it : dataPacket->structures) {
		createTypeByDescIfNotExists(it.type);
	}

	for (auto it : dataPacket->classes) {
		createTypeByDescIfNotExists(it.structType.type);
	}

	for (auto it : dataPacket->signatures) {
		createTypeByDescIfNotExists(it.type);
	}

	m_typedefTypeMapper->load(dataPacket);
	m_enumTypeMapper->load(dataPacket);
	m_structureTypeMapper->load(dataPacket);
	m_classTypeMapper->load(dataPacket);
	m_signatureTypeMapper->load(dataPacket);
}

void markObjectAsSynced(SyncContext* ctx, DataType::UserDefinedType* type) {
	SQLite::Statement query(*ctx->m_db, "UPDATE sda_types SET ghidra_sync_id=?1 WHERE id=?2");
	query.bind(1, ctx->m_syncId);
	query.bind(2, type->getId());
	query.exec();
}

void DataTypeMapper::upsert(SyncContext* ctx, IObject* obj) {
	auto type = dynamic_cast<DataType::UserDefinedType*>(obj);
	markObjectAsSynced(ctx, type);
}

void DataTypeMapper::remove(SyncContext* ctx, IObject* obj) {
	auto type = dynamic_cast<DataType::UserDefinedType*>(obj);
	ctx->m_dataPacket->removed_datatypes.push_back(type->getGhidraId());
	markObjectAsSynced(ctx, type);
}

datatype::SDataType DataTypeMapper::buildDesc(DataType::UserDefinedType* type) {
	datatype::SDataType typeDesc;
	typeDesc.__set_id(type->getGhidraId());
	typeDesc.__set_group((datatype::DataTypeGroup::type)type->getGroup());
	typeDesc.__set_size(type->getSize());
	typeDesc.__set_name(type->getName());
	typeDesc.__set_comment(type->getComment());
	return typeDesc;
}

shared::STypeUnit DataTypeMapper::buildTypeUnitDesc(DataTypePtr dataType) const
{
	shared::STypeUnit typeUnitDesc;
	typeUnitDesc.__set_typeId(m_typeManager->getGhidraId(dataType->getType()));
	for (auto lvl : dataType->getPointerLevels()) {
		typeUnitDesc.pointerLvls.push_back((int16_t)lvl);
	}
	return typeUnitDesc;
}

DataTypePtr DataTypeMapper::getTypeByDesc(const shared::STypeUnit& desc) const
{
	std::list<int> ptr_levels;
	for (auto lvl : desc.pointerLvls) {
		ptr_levels.push_back(lvl);
	}
	
	auto type = m_typeManager->findTypeByGhidraId(desc.typeId);
	if (type == nullptr)
		type = m_typeManager->getFactory().getDefaultType();
	return std::make_shared<DataType::Unit>(type, ptr_levels);
}

void DataTypeMapper::changeUserTypeByDesc(DataType::UserDefinedType* type, const datatype::SDataType& typeDesc) {
	type->setName(typeDesc.name);
	if (typeDesc.comment != "{pull}") {
		type->setComment(typeDesc.comment);
	}
}
