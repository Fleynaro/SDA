#include "GhidraClassTypeMapper.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace CE::Ghidra;

ClassTypeMapper::ClassTypeMapper(StructureTypeMapper* structTypeMapper)
	: m_structTypeMapper(structTypeMapper)
{}

void ClassTypeMapper::load(packet::SDataFullSyncPacket* dataPacket) {
	for (auto classDesc : dataPacket->classes) {
		const auto type = m_structTypeMapper->m_dataTypeMapper->m_typeManager->findTypeByGhidraId(classDesc.structType.type.id);
		if (type == nullptr)
			throw std::exception("item not found");
		if (const auto Class = dynamic_cast<DataType::Class*>(type)) {
			changeClassByDesc(Class, classDesc);
		}
	}
}

void ClassTypeMapper::upsert(SyncContext* ctx, IObject* obj) {
	const auto type = dynamic_cast<DataType::Class*>(obj);
	ctx->m_dataPacket->classes.push_back(buildDesc(type));
	m_structTypeMapper->m_dataTypeMapper->upsert(ctx, obj);
}

void ClassTypeMapper::remove(SyncContext* ctx, IObject* obj) {
	m_structTypeMapper->remove(ctx, obj);
}

datatype::SDataTypeClass ClassTypeMapper::buildDesc(DataType::Class* Class) const
{
	datatype::SDataTypeClass classDesc;
	classDesc.__set_structType(m_structTypeMapper->buildDesc(Class));
	return classDesc;
}

void ClassTypeMapper::changeClassByDesc(DataType::Class* Class, const datatype::SDataTypeClass& classDesc) const
{
	m_structTypeMapper->changeStructureByDesc(Class, classDesc.structType);
}
