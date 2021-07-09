#pragma once
#include <Windows.h>

namespace CE
{
	class Address
	{
		void* m_addr;

	public:
		Address(void* addr)
			: m_addr(addr)
		{}

		[[nodiscard]] bool canBeRead() const;

		[[nodiscard]] HMODULE getModuleHandle() const;

		[[nodiscard]] MEMORY_BASIC_INFORMATION getInfo() const;

		[[nodiscard]] void* getAddress() const;

		template<typename T>
		T& get();

		enum ProtectFlags {
			No			= 0,
			Read		= 1,
			Write		= 2,
			Execute		= 4
		};

		void setProtect(ProtectFlags flags, int size = 1) const;

		[[nodiscard]] ProtectFlags getProtect() const;
	};

	template<typename T>
	inline T& Address::get() {
		return *static_cast<T*>(m_addr);
	}
};