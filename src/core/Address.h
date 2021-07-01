#pragma once
#include <Windows.h>
#include <datatypes/TypeUnit.h>
#include <utilities/Iterator.h>

namespace CE
{
	class Address
	{
		void* m_addr;

	public:
		Address(void* addr)
			: m_addr(addr)
		{}

		bool canBeRead();

		HMODULE getModuleHandle();

		MEMORY_BASIC_INFORMATION getInfo();

		void* getAddress() {
			return m_addr;
		}

		template<typename T>
		T& get();

		enum ProtectFlags {
			No			= 0,
			Read		= 1,
			Write		= 2,
			Execute		= 4
		};

		void setProtect(ProtectFlags flags, int size = 1);

		ProtectFlags getProtect();
	};

	template<typename T>
	inline T& Address::get() {
		return *(T*)m_addr;
	}
};