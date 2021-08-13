#include "ImageReader.h"

#include <utility>

CE::DataPointerReader::DataPointerReader(uint8_t* data, int size, bool isDelete)
	: m_data(data), m_size(size), m_isDelete(isDelete)
{}

CE::DataPointerReader::~DataPointerReader() {
	if (m_isDelete)
		delete[] m_data;
}

void CE::DataPointerReader::read(uint64_t offset, std::vector<uint8_t>& data) {
	const auto size = std::min(getSize() - offset, data.size());
	std::copy_n(&m_data[offset], size, data.begin());
}

int CE::DataPointerReader::getSize() {
	return m_size;
}

CE::VectorReader::VectorReader(std::vector<uint8_t> data)
	: m_data(std::move(data))
{}

void CE::VectorReader::read(uint64_t offset, std::vector<uint8_t>& data) {
	const auto size = std::min(getSize() - offset, data.size());
	std::copy_n(m_data.begin() + offset, size, data.begin());
}

int CE::VectorReader::getSize() {
	return static_cast<int>(m_data.size());
}

CE::DebugReader::DebugReader(IDebugSession* debugSession, DebugModule debugModule)
	: m_debugSession(debugSession), m_module(std::move(debugModule))
{}

void CE::DebugReader::read(uint64_t offset, std::vector<uint8_t>& data) {
	if(m_debugSession->isSuspended()) {
		m_debugSession->readMemory(m_module.m_baseAddress + offset, data);
	} else {
		if (m_isCacheEnabled) {
			const auto size = std::min(getSize() - offset, data.size());
			std::copy_n(m_cache.begin() + offset, size, data.begin());
		}
	}
}

int CE::DebugReader::getSize() {
	return m_module.m_size;
}

void CE::DebugReader::updateCache() {
	m_cache.resize(m_module.m_size);
	m_debugSession->readMemory(m_module.m_baseAddress, m_cache);
}

void CE::DebugReader::removeCache() {
	m_cache.resize(0x0);
}

void CE::DebugReader::setCacheEnabled(bool toggle) {
	m_isCacheEnabled = toggle;
}
