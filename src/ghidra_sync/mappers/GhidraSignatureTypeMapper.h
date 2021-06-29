#pragma once
#include "GhidraDataTypeMapper.h"
#include <Code/Type/FunctionSignature.h>

namespace CE::Ghidra
{
	class SignatureTypeMapper : public IMapper
	{
	public:
		SignatureTypeMapper(DataTypeMapper* dataTypeMapper);

		void load(packet::SDataFullSyncPacket* dataPacket) override;

		void upsert(SyncContext* ctx, IObject* obj) override;

		void remove(SyncContext* ctx, IObject* obj) override;

		datatype::SDataTypeSignature buildDesc(DataType::FunctionSignature* sig) {
			datatype::SDataTypeSignature sigDesc;
			sigDesc.__set_type(m_dataTypeMapper->buildDesc(sig));
			sigDesc.__set_returnType(m_dataTypeMapper->buildTypeUnitDesc(sig->getReturnType()));

			for (auto param : sig->getParameters()) {
				datatype::SFunctionArgument argDesc;
				argDesc.__set_name(param->getName());
				argDesc.__set_type(m_dataTypeMapper->buildTypeUnitDesc(param->getDataType()));
				sigDesc.arguments.push_back(argDesc);
			}
			return sigDesc;
		}

		void changeSignatureByDesc(DataType::FunctionSignature* sig, const datatype::SDataTypeSignature& sigDesc) {
			m_dataTypeMapper->changeUserTypeByDesc(sig, sigDesc.type);
			sig->setReturnType(m_dataTypeMapper->getTypeByDesc(sigDesc.returnType));
			sig->deleteAllParameters();
			for (auto argDesc : sigDesc.arguments) {
				sig->addParameter(argDesc.name, m_dataTypeMapper->getTypeByDesc(argDesc.type));
			}
		}

	private:
		DataTypeMapper* m_dataTypeMapper;
	};
};