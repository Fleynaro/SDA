#include "Core/Image/ImageReader.h"
#include <fstream>

using namespace sda;

PointerImageReader::PointerImageReader(uint8_t* data, size_t size)
    : m_data(data), m_size(size)
{}

void PointerImageReader::readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) {
    const auto size = std::min(getImageSize() - offset, data.size());
	std::copy_n(&m_data[offset], size, data.begin());
}

size_t PointerImageReader::getImageSize() {
    return m_size;
}

VectorImageReader::VectorImageReader(const std::vector<uint8_t>& data)
    : m_data(std::move(data))
{}

void VectorImageReader::readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) {
    const auto size = std::min(getImageSize() - offset, data.size());
    std::copy_n(m_data.begin() + offset, size, data.begin());
}

size_t VectorImageReader::getImageSize() {
    return m_data.size();
}

FileImageReader::FileImageReader(const std::filesystem::path& pathToImgFile)
    : m_pathToImgFile(pathToImgFile)
{}

void FileImageReader::readFile() {
    std::ifstream file(m_pathToImgFile, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + m_pathToImgFile.string());
    
    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    m_reader = std::make_unique<VectorImageReader>(std::move(data));
}

void FileImageReader::readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) {
    m_reader->readBytesAtOffset(offset, data);
}

size_t FileImageReader::getImageSize() {
    return m_reader->getImageSize();
}

void FileImageReader::serialize(boost::json::object& data) const {
    data["type"] = Name;
    data["path_to_img_file"] = m_pathToImgFile.string();
}

void FileImageReader::deserialize(boost::json::object& data) {
    m_pathToImgFile = std::string(data["path_to_img_file"].get_string());
    readFile();
}