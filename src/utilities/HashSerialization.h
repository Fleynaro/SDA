#pragma once
#include <stdint.h>
#include <string>

// Hash (with encapsulated operations <<, +)
class HS
{
public:
	using Value = uint64_t;
	const static Value DefaultSeed = 1125899906842597L;
private:
	Value m_hashValue = DefaultSeed;
public:
	HS(Value value = DefaultSeed)
		: m_hashValue(value)
	{}

	// integral
	template<class T>
	HS operator+(const T& value) const {
		Value value64 = 0;
		reinterpret_cast<T&>(value64) = value;
		return HS(m_hashValue + value64);
	}

	template<class T>
	HS operator<<(const T& value) const {
		auto hash = *this + value;
		hash.m_hashValue += hash.m_hashValue << 5;
		return hash;
	}

	// string
	HS operator+(const std::string& string) const {
		return *this + HashString(string);
	}

	HS operator<<(const std::string& string) const {
		return *this << HashString(string);
	}

	// another hash
	HS operator+(const HS& hs) const {
		return *this + hs.m_hashValue;
	}

	HS operator<<(const HS& hs) const {
		return *this << hs.m_hashValue;
	}

	bool operator<(const HS& hs) const {
		return m_hashValue < hs.m_hashValue;
	}
	
	// get hash as integer value
	Value getHashValue() const {
		return m_hashValue;
	}

private:
	static HS HashString(const std::string& string) {
		HS hs;
		for (const auto& ch : string) {
			hs = hs << ch;
		}
		return hs;
	}
};