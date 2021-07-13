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
	HS(Value seed = DefaultSeed)
		: m_hashValue(seed)
	{}

	template<class T>
	HS operator+(T value) const {
		return HS(m_hashValue + static_cast<Value>(value));
	}

	template<class T>
	HS operator<<(T value) const {
		return *this + static_cast<uint64_t>(value) * 31;
	}
	
	HS operator+(float value) const {
		return *this + reinterpret_cast<uint32_t&>(value);
	}

	HS operator<<(float value) const {
		return *this << reinterpret_cast<uint32_t&>(value);
	}

	HS operator+(double value) const {
		return *this + reinterpret_cast<uint64_t&>(value);
	}

	HS operator<<(double value) const {
		return *this << reinterpret_cast<uint64_t&>(value);
	}
	
	HS operator+(const std::string& string) const {
		return *this + HashString(string);
	}

	HS operator<<(const std::string& string) const {
		return *this << HashString(string);
	}

	HS operator+(const HS& hs) const {
		return *this + hs.m_hashValue;
	}

	HS operator<<(const HS& hs) const {
		return *this << hs.m_hashValue;
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