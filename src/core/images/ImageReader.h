#pragma once
#include "debugger/DebugSession.h"
#include <cstdint>
#include <vector>

namespace CE
{
	// allow to read data from array, vector or even other process
	class IReader
	{
	public:
		virtual void read(uint64_t offset, std::vector<uint8_t>& data) = 0;

		virtual int getSize() = 0;
	};
	
	class SimpleReader : public IReader
	{
		uint8_t* m_data;
		int m_size;
		bool m_isDelete;
	public:
		SimpleReader(uint8_t* data, int size, bool isDelete = true);

		~SimpleReader();

		void read(uint64_t offset, std::vector<uint8_t>& data) override;

		int getSize() override;
	};

	class VectorReader : public SimpleReader
	{
		std::vector<uint8_t> m_data;
	public:
		VectorReader(std::vector<uint8_t> data);
	};

	class IDebugSession;
	class DebugReader : public IReader
	{
		IDebugSession* m_debugSession;
		DebugModule m_module;
	public:
		DebugReader(IDebugSession* debugSession, DebugModule debugModule);

		void read(uint64_t offset, std::vector<uint8_t>& data) override;

		int getSize() override;
	};
};