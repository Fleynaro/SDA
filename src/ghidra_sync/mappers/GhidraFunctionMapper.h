#pragma once
#include <ghidra_sync/GhidraAbstractMapper.h>
#include <Function.h>

namespace CE {
	class FunctionManager;
};

namespace CE::Ghidra
{
	using namespace ghidra;
	using namespace function;

	class DataTypeMapper;

	class FunctionMapper : public IMapper
	{
	public:
		FunctionMapper(FunctionManager* functionManager, DataTypeMapper* dataTypeMapper);

		void load(packet::SDataFullSyncPacket* dataPacket) override;

		void upsert(SyncContext* ctx, IObject* obj) override;

		void remove(SyncContext* ctx, IObject* obj) override;

	private:
		FunctionManager* m_functionManager;
		DataTypeMapper* m_dataTypeMapper;

		void changeFunctionByDesc(Function* function, const SFunction& funcDesc);

		SFunction buildDesc(Function* function);
	};
};