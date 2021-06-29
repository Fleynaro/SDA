#pragma once
#include <main.h>

class ObjectHash
{
public:
	using Hash = int64_t;

	ObjectHash(Hash hash = 0L, std::string hashContent = "")
		: m_hash(hash), m_hashContent(hashContent)
	{}

	void addValue(std::string value);

	void addValue(int value);

	void addValue(int64_t value);

	Hash getHash();

	void join(ObjectHash& hash);

	void add(ObjectHash& hash);

	static Hash hash(std::string string);
private:
	std::string m_hashContent;
	Hash m_hash;
};