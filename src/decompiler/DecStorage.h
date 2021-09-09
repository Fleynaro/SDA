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

		Storage(StorageType storageType = STORAGE_NONE, int regGenericId = 0, int64_t offset = 0);

		Storage(const PCode::Register& reg);

		StorageType getType() const;

		// generic id + index
		int getRegId() const;

		// generic id only
		int getRegGenericId() const;

		int64_t getOffset() const;
	
	private:
		StorageType m_storageType;
		int m_regId; // generic id + index
		int64_t m_offset;
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

	extern bool GetIndex_FASTCALL(const PCode::Register& reg, int64_t offset, int& paramIdx, bool& isFloating);
};