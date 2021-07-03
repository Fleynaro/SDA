#include "BufferStreamRecorder.h"

BufferIterator::BufferIterator(Buffer* buffer)
	: m_buffer(buffer), m_bufferStream(buffer)
{
	countSize();
}

bool BufferIterator::hasNext() {//MYTODO: check offset
	return m_curSize > 0 && getOffset() + m_curSize <= m_buffer->getContentOffset();
}

Buffer::Stream BufferIterator::getStream() {
	Buffer::Stream bufferStream = m_bufferStream;
	m_bufferStream.move(m_curSize);
	countSize();
	return bufferStream;
}

int BufferIterator::getOffset() {
	return m_bufferStream.getOffset();
}

void BufferIterator::countSize() {
	m_curSize = m_bufferStream.read<int>();
}

StreamRecord::StreamRecord(Buffer::Stream* bufferStream, StreamRecordWriter* streamRecordWriter)
	: m_bufferStream(bufferStream), m_streamRecordWriter(streamRecordWriter)
{}

void StreamRecord::write() {
	writeHeader();
	m_streamRecordWriter->setBufferStream(m_bufferStream);
	m_streamRecordWriter->write();
	writeEnd();
}

void StreamRecord::writeHeader() {
	m_size = m_bufferStream->getNext<int>();
	m_bufferStream->write(0);
}

void StreamRecord::writeEnd() {
	*m_size = m_streamRecordWriter->getWrittenLength();
}

StreamRecordWriter::StreamRecordWriter()
{}

int StreamRecordWriter::getWrittenLength() {
	return getStream().getOffset();
}

Buffer::Stream& StreamRecordWriter::getStream() {
	return m_bufferStream;
}

void StreamRecordWriter::setBufferStream(Buffer::Stream bufferStream) {
	m_bufferStream = bufferStream;
}
