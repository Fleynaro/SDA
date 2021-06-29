#pragma once
#include <main.h>

class BitStream
{
public:
	BitStream() {
		m_bytes.push_back(0);
	}
	BitStream(BYTE* data, int size) {
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

	void setData(BYTE* data, int size);

	BYTE* getData();

	int getSize();

	void resetPointer();
private:
	inline void inc();

	int m_curByte;
	int m_curBit;
	std::vector<BYTE> m_bytes;
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
