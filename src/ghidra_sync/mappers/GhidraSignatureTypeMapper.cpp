#include "GhidraSignatureTypeMapper.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace CE::Ghidra;

SignatureTypeMapper::SignatureTypeMapper(DataTypeMapper* dataTypeMapper)
	: m_dataTypeMapper(dataTypeMapper)
{}

void SignatureTypeMapper::load(packet::SDataFullSyncPacket* dataPacket) {
	for (auto sigDesc : dataPacket->signatures) {
		auto type = m_dataTypeMapper->m_typeManager->findTypeByGhidraId(sigDesc.type.id);
		if (type == nullptr)
			throw std::exception("item not found");
		if (auto sigDef = dynamic_cast<DataType::FunctionSignature*>(type)) {
			changeSignatureByDesc(sigDef, sigDesc);
		}
	}
}

void SignatureTypeMapper::upsert(SyncContext* ctx, IObject* obj) {
	auto type = dynamic_cast<DataType::FunctionSignature*>(obj);
	ctx->m_dataPacket->signatures.push_back(buildDesc(type));
	m_dataTypeMapper->upsert(ctx, obj);
}

void SignatureTypeMapper::remove(SyncContext* ctx, IObject* obj) {
	m_dataTypeMapper->remove(ctx, obj);
}

