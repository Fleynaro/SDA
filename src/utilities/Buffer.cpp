#include "Buffer.h"

void Buffer::init(int size) {
	m_header.m_contentSize = size;
	m_header.m_currentOffset = 0;
}

Buffer* Buffer::Create(int size) {
    auto buffer = (Buffer*)(new BYTE[size]);
    buffer->init(size);
    return buffer;
}

void Buffer::Destroy(Buffer* buffer) {
    delete[](BYTE*)buffer;
}

BYTE* Buffer::getData() {
    return (BYTE*)&m_header;
}

BYTE* Buffer::getContent() {
    return (BYTE*)((std::uintptr_t) & m_header + sizeof(m_header));
}

int Buffer::getSize() {
    return sizeof(m_header) + m_header.m_contentSize;
}

int Buffer::getContentSize() {
    return m_header.m_contentSize;
}

int Buffer::getContentOffset() {
    return m_header.m_currentOffset;
}

int Buffer::getFreeSpaceSize() {
    return getContentSize() - getContentOffset();
}

Buffer::Stream::Stream(Buffer* buffer)
    : m_buffer(buffer)
{
    setNext(m_data = m_buffer->getContent());
}

Buffer::Stream::Stream(Stream* bufferStream)
    : m_bufferStream(bufferStream), m_buffer(bufferStream->m_buffer)
{
    setNext(m_data = m_bufferStream->getNext());
}


Buffer::Stream& Buffer::Stream::writeFrom(void* addr, int size) {
    if (!isFree(size)) {
        throw BufferOverflowException();
        return *this;
    }
    memcpy_s(m_curData, m_buffer->getFreeSpaceSize(), addr, size);
    move(size, true);
    return *this;
}

bool Buffer::Stream::isFree(int size) {
    return m_buffer->getFreeSpaceSize() >= size;
}

void Buffer::Stream::move(int bytes, bool write) {
    m_curData += bytes;

    if (write) {
        if (m_buffer->m_header.m_currentOffset < getOffset())
            m_buffer->m_header.m_currentOffset = getOffset();
    }

    if (m_bufferStream != nullptr)
        m_bufferStream->move(bytes, write);
}

int Buffer::Stream::getOffset() {
    return (int)((std::uintptr_t)m_curData - (std::uintptr_t)m_data);
}
