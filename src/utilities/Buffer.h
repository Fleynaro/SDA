#pragma once
#include <stdint.h>
#include <stdexcept>

class BufferOverflowException : public std::exception
{};

class Buffer
{
public:
    struct Header {
        int m_contentSize;
        int m_currentOffset;
    };
    Header m_header;

private:
    void init(int size);

public:
    static Buffer* Create(int size);

    static void Destroy(Buffer* buffer);

    int8_t* getData();

    int8_t* getContent();

    int getSize();

    int getContentSize();

    int getContentOffset();

    int getFreeSpaceSize();

    class Stream {
    public:
        Stream() = default;

        Stream(Buffer* buffer);

        Stream(Stream* bufferStream);

        template<typename T = int8_t>
        inline Stream& write(const T& data);

        Stream& writeFrom(void* addr, int size);

        template<typename T = int8_t>
        inline T& read();

        template<typename T = int8_t>
        inline T* readPtr(int size = sizeof(T));

        bool isFree(int size);

        void move(int bytes, bool write = false);

        template<typename T = int8_t>
        inline T* getNext();

        template<typename T = int8_t>
        inline void setNext(T* ptr);

        int getOffset();
    private:
        Buffer* m_buffer;
        int8_t* m_data;
        int8_t* m_curData;

        Stream* m_bufferStream = nullptr;
    };

    friend class Stream;
};

template<typename T>
inline Buffer::Stream& Buffer::Stream::write(const T& data) {
    if (!isFree(sizeof(T))) {
        throw BufferOverflowException();
        return *this;
    }
    (T&)*m_curData = data;
    move(sizeof(T), true);
    return *this;
}

template<typename T>
inline T& Buffer::Stream::read() {
    return *readPtr<T>();
}

template<typename T>
inline T* Buffer::Stream::readPtr(int size) {
    if (!isFree(size))
        throw BufferOverflowException();
    auto data = (T*)m_curData;
    m_curData += size;
    return data;
}

template<typename T>
inline T* Buffer::Stream::getNext() {
    if (!isFree(sizeof(T))) {
        throw BufferOverflowException();
    }
    return (T*)m_curData;
}

template<typename T>
inline void Buffer::Stream::setNext(T* ptr) {
    m_curData = (int8_t*)ptr;
}
