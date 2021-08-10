#include "ImageReader.h"

#include <utility>

CE::SimpleReader::SimpleReader(uint8_t* data, int size, bool isDelete)
	: m_data(data), m_size(size), m_isDelete(isDelete)
{}

CE::SimpleReader::~SimpleReader() {
	if (m_isDelete)
		delete[] m_data;
}

void CE::SimpleReader::read(uint64_t offset, std::vector<uint8_t>& data) {
	const auto size = std::min(m_size - offset, data.size());
	std::copy_n(m_data, size, data.begin());
}

int CE::SimpleReader::getSize() {
	return m_size;
}

CE::VectorReader::VectorReader(std::vector<uint8_t> data)
	: m_data(std::move(data)), SimpleReader(m_data.data(), m_data.size(), false)
{}

CE::DebugReader::DebugReader(IDebugSession* debugSession, DebugModule debugModule)
	: m_debugSession(debugSession), m_module(std::move(debugModule))
{}

void CE::DebugReader::read(uint64_t offset, std::vector<uint8_t>& data) {
	m_debugSession->readMemory(m_module.m_baseAddress + offset, data);
}

int CE::DebugReader::getSize() {
	return m_module.m_size;
}
