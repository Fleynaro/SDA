#pragma once
#include "Buffer.h"

class StreamRecordWriter
{
public:
	StreamRecordWriter();

	int getWrittenLength();

	virtual void write() = 0;

	Buffer::Stream& getStream();

	void setBufferStream(Buffer::Stream bufferStream);
private:
	Buffer::Stream m_bufferStream;
};

class StreamRecord
{
public:
	StreamRecord(Buffer::Stream* bufferStream, StreamRecordWriter* streamRecordWriter);

	void write();
private:
	void writeHeader();

	void writeEnd();
protected:
	Buffer::Stream* m_bufferStream;
	StreamRecordWriter* m_streamRecordWriter;
	int* m_size;
};

class BufferIterator {
public:
	BufferIterator(Buffer* buffer);

	bool hasNext();

	Buffer::Stream getStream();

	int getOffset();
private:
	Buffer* m_buffer;
	Buffer::Stream m_bufferStream;
	int m_curSize;

	void countSize();
};