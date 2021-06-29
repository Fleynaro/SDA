#include "BitStream.h"

void BitStream::writeBit(bool bit)
{
	m_bytes[m_curByte] = m_bytes[m_curByte] & ~(0b1 << m_curBit) | (bit << m_curBit);
	inc();
}

void BitStream::write(const void* src, int size) {
	BYTE* data = (BYTE*)src;
	for (int i = 0; i < size; i++)
		write(data[i]);
}

bool BitStream::readBit()
{
	bool result = m_bytes[m_curByte] >> m_curBit & 0b1;
	inc();
	return result;
}

void BitStream::read(void* dst, int size) {
	BYTE* data = (BYTE*)dst;
	for (int i = 0; i < size; i++)
		data[i] = read<BYTE>();
}

void BitStream::setData(BYTE* data, int size) {
	for (int i = 0; i < size; i++) {
		m_bytes.push_back(data[i]);
	}
}

BYTE* BitStream::getData() {
	return m_bytes.data();
}

int BitStream::getSize() {
	return m_curByte;
}

void BitStream::resetPointer() {
	m_curByte = 0;
	m_curBit = 0;
}

void BitStream::inc() {
	if (++m_curBit == 0x8 * sizeof(BYTE)) {
		m_curByte++;
		m_curBit = 0;
		if (m_curByte == m_bytes.size())
			m_bytes.push_back(0);
	}
}
