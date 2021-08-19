#pragma once
#include "PCode/DecPCode.h"

namespace CE::Decompiler
{
	class Storage
	{
	public:
		enum StorageType {
			STORAGE_NONE,
			STORAGE_REGISTER,
			STORAGE_STACK,
			STORAGE_GLOBAL
		};

		Storage(StorageType storageType = STORAGE_NONE, int registerId = 0, int64_t offset = 0);

		Storage(const PCode::Register& reg);

		StorageType getType() const;

		int getRegisterId() const;

		int64_t getOffset() const;
	
	private:
		StorageType m_storageType;
		int m_registerId;
		int64_t m_offset;
	};

	struct StoragePath
	{
		PCode::Register m_register;
		std::list<int64_t> m_offsets;
	};

	struct ParameterInfo
	{
		int m_index = 0;
		int m_size = 0;
		Storage m_storage;

		ParameterInfo() = default;

		ParameterInfo(int index, int size, Storage storage)
			: m_index(index), m_size(size), m_storage(storage)
		{}

		int getIndex() const;
	};
	using ReturnInfo = ParameterInfo;

	class FunctionCallInfo
	{
		std::list<ParameterInfo> m_paramInfos;
	public:
		FunctionCallInfo(std::list<ParameterInfo> paramInfos);

		std::list<ParameterInfo>& getParamInfos();

		ParameterInfo findParamInfoByIndex(int idx);

		ReturnInfo getReturnInfo();

		int findIndex(const PCode::Register& reg, int64_t offset);
	};

	extern int GetIndex_FASTCALL(const PCode::Register& reg, int64_t offset);
};