#include "GhidraTypedefTypeMapper.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace CE::Ghidra;

TypedefTypeMapper::TypedefTypeMapper(DataTypeMapper* dataTypeMapper)
	: m_dataTypeMapper(dataTypeMapper)
{}

void TypedefTypeMapper::load(packet::SDataFullSyncPacket* dataPacket) {
	for (auto typedefDesc : dataPacket->typedefs) {
		const auto type = m_dataTypeMapper->m_typeManager->findTypeByGhidraId(typedefDesc.type.id);
		if (type == nullptr)
			throw std::exception("item not found");
		if (const auto typeDef = dynamic_cast<DataType::Typedef*>(type)) {
			changeTypedefByDesc(typeDef, typedefDesc);
		}
	}
}

void TypedefTypeMapper::upsert(SyncContext* ctx, IObject* obj) {
	const auto type = dynamic_cast<DataType::Typedef*>(obj);
	ctx->m_dataPacket->typedefs.push_back(buildDesc(type));
	m_dataTypeMapper->upsert(ctx, obj);
}

void TypedefTypeMapper::remove(SyncContext* ctx, IObject* obj) {
	m_dataTypeMapper->remove(ctx, obj);
}

datatype::SDataTypeTypedef TypedefTypeMapper::buildDesc(DataType::Typedef* Typedef) const
{
	datatype::SDataTypeTypedef typedefDesc;
	typedefDesc.__set_type(m_dataTypeMapper->buildDesc(Typedef));
	typedefDesc.__set_refType(m_dataTypeMapper->buildTypeUnitDesc(Typedef->getRefType()));
	return typedefDesc;
}

void TypedefTypeMapper::changeTypedefByDesc(DataType::Typedef* Typedef, const datatype::SDataTypeTypedef& typedefDesc) const
{
	m_dataTypeMapper->changeUserTypeByDesc(Typedef, typedefDesc.type);
	Typedef->setRefType(m_dataTypeMapper->getTypeByDesc(typedefDesc.refType));
}
