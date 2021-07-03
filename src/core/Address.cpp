#include "Address.h"
#include <cstdint>


using namespace CE;

bool Address::canBeRead() const
{
	__try {
		int8_t firstByte = *static_cast<int8_t*>(m_addr);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return true;
}

HMODULE Address::getModuleHandle() const
{
	return static_cast<HMODULE>(getInfo().AllocationBase);
}

MEMORY_BASIC_INFORMATION Address::getInfo() const
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(m_addr, &mbi, sizeof(mbi));
	return mbi;
}

void* Address::getAddress() const
{
	return m_addr;
}

void Address::setProtect(ProtectFlags flags, int size) const
{
	DWORD new_ = PAGE_NOACCESS;
	DWORD old_;

	switch (flags)
	{
	case Read:
		new_ = PAGE_READONLY;
		break;
	case Write:
	case Write | Read:
		new_ = PAGE_READWRITE;
		break;
	case Execute:
		new_ = PAGE_EXECUTE;
		break;
	case Execute | Read:
		new_ = PAGE_EXECUTE_READ;
		break;
	case Execute | Write:
	case Execute | Read | Write:
		new_ = PAGE_EXECUTE_READWRITE;
		break;
	case No: break;
	default: ;
	}

	VirtualProtect(m_addr, size, new_, &old_);
}

Address::ProtectFlags Address::getProtect() const
{
	const auto protect = getInfo().Protect;
	DWORD result = 0;

	if (protect & PAGE_READONLY)
		result |= Read;
	if (protect & PAGE_READWRITE)
		result |= Read | Write;
	if (protect & PAGE_EXECUTE)
		result |= Execute;
	if (protect & PAGE_EXECUTE_READ)
		result |= Execute | Read;
	if (protect & PAGE_EXECUTE_READWRITE)
		result |= Execute | Read | Write;

	return static_cast<ProtectFlags>(result);
}
