#pragma once
#include <stdint.h>
#include <vector>

class BitStream
{
public:
	BitStream() {
		m_bytes.push_back(0);
	}

	BitStream(int8_t* data, int size) {
		setData(data, size);
	}

	void writeBit(bool bit);

	template<typename T>
	void write(T value);

	void write(const void* src, int size);

	bool readBit();

	template<typename T>
	T read();

	void read(void* dst, int size);

	void setData(int8_t* data, int size);

	int8_t* getData();

	int getSize();

	void resetPointer();
private:
	inline void inc();

	int m_curByte = 0;
	int m_curBit = 0;
	std::vector<int8_t> m_bytes;
};

template<typename T>
void BitStream::write(T value)
{
	for (int i = 0; i < sizeof(T) * 0x8; i++) {
		writeBit(value >> i & 0b1);
	}
}

template<typename T>
T BitStream::read()
{
	T result = 0;
	for (int i = 0; i < sizeof(T) * 0x8; i++) {
		result |= readBit() << i;
	}
	return result;
}
