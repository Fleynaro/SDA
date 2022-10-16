#include "SDA/Core/Image/ImageRW.h"
#include <fstream>

using namespace sda;

PointerImageRW::PointerImageRW(uint8_t* data, size_t size)
    : m_data(data), m_size(size)
{}

void PointerImageRW::readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) {
    const auto size = std::min(getImageSize() - offset, data.size());
	std::copy_n(&m_data[offset], size, data.begin());
}

void PointerImageRW::writeBytesAtOffset(Offset offset, const std::vector<uint8_t>& data) {
    if (offset + data.size() > getImageSize())
        throw std::runtime_error("Offset out of bounds");
    std::copy_n(data.begin(), data.size(), &m_data[offset]);
}

size_t PointerImageRW::getImageSize() {
    return m_size;
}

VectorImageRW::VectorImageRW(const std::vector<uint8_t>& data)
    : m_data(std::move(data))
{}

void VectorImageRW::readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) {
    const auto size = std::min(getImageSize() - offset, data.size());
    std::copy_n(m_data.begin() + offset, size, data.begin());
}

void VectorImageRW::writeBytesAtOffset(Offset offset, const std::vector<uint8_t>& data) {
    if (offset + data.size() > getImageSize())
        throw std::runtime_error("Offset out of bounds");
    std::copy_n(data.begin(), data.size(), m_data.begin() + offset);
}

size_t VectorImageRW::getImageSize() {
    return m_data.size();
}

const std::vector<uint8_t>& VectorImageRW::getData() const {
    return m_data;
}

FileImageRW::FileImageRW(const std::filesystem::path& pathToImgFile)
    : m_pathToImgFile(pathToImgFile)
{}

void FileImageRW::readFile() {
    std::ifstream file(m_pathToImgFile, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + m_pathToImgFile.string());
    
    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    m_rw = std::make_unique<VectorImageRW>(std::move(data));
}

void FileImageRW::saveFile() {
    std::ofstream file(m_pathToImgFile, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + m_pathToImgFile.string());

    auto& data = m_rw->getData();
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void FileImageRW::readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) {
    m_rw->readBytesAtOffset(offset, data);
}

void FileImageRW::writeBytesAtOffset(Offset offset, const std::vector<uint8_t>& data) {
    m_rw->writeBytesAtOffset(offset, data);
}

size_t FileImageRW::getImageSize() {
    return m_rw->getImageSize();
}

void FileImageRW::serialize(boost::json::object& data) const {
    data["type"] = Name;
    data["path_to_img_file"] = m_pathToImgFile.string();
}

void FileImageRW::deserialize(boost::json::object& data) {
    m_pathToImgFile = std::string(data["path_to_img_file"].get_string().c_str());
    readFile();
}