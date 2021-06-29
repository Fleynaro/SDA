#pragma once
#include <GhidraSync/GhidraAbstractMapper.h>
#include <Code/Function/Function.h>

namespace CE {
	class FunctionManager;
};

namespace CE::Ghidra
{
	using namespace ghidra;
	using namespace ghidra::function;

	class DataTypeMapper;

	class FunctionMapper : public IMapper
	{
	public:
		FunctionMapper(CE::FunctionManager* functionManager, DataTypeMapper* dataTypeMapper);

		void load(packet::SDataFullSyncPacket* dataPacket) override;

		void upsert(SyncContext* ctx, IObject* obj) override;

		void remove(SyncContext* ctx, IObject* obj) override;

	private:
		CE::FunctionManager* m_functionManager;
		DataTypeMapper* m_dataTypeMapper;

		void changeFunctionByDesc(Function* function, const function::SFunction& funcDesc);

		function::SFunction buildDesc(Function* function);
	};
};