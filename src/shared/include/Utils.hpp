#pragma once
#include <memory>

// Decalre unique pointer for a class.
#define DECL_UNIQUE_PTR(ClassName) using ClassName ## Ptr = std::unique_ptr<ClassName>;

// Decalre unique pointer for a class before its definition.
#define DECL_UNIQUE_PTR_BEFORE(ClassName) \
	class ClassName; \
	DECL_UNIQUE_PTR(ClassName)

// Decalre shared pointer for a class.
#define DECL_SHARED_PTR(ClassName) using ClassName ## Ptr = std::shared_ptr<ClassName>;

// Decalre shared pointer for a class before its definition.
#define DECL_SHARED_PTR_BEFORE(ClassName) \
	class ClassName; \
	DECL_SHARED_PTR(ClassName)

// Make dynamic cast to a class.
template <typename To, typename From> 
std::unique_ptr<To> dynamic_unique_cast(std::unique_ptr<From>&& p) {
	if (To* cast = dynamic_cast<To*>(p.get()))
	{
		std::unique_ptr<To> result(cast);
		p.release();
		return result;
	}
	return std::unique_ptr<To>(nullptr);
}