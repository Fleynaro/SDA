#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <Core/Serialization.h>

namespace sda
{
    // Image reader interface
    class IImageReader
	{
	public:
        // Read bytes from the image at the given offset
		virtual void readBytesAtOffset(size_t offset, std::vector<uint8_t>& bytes) = 0;

        // Get the size of the image
		virtual size_t getImageSize() = 0;
	};

    class PointerImageReader : public IImageReader
	{
        uint8_t* m_data;
		size_t m_size;
	public:
		PointerImageReader(uint8_t* data, size_t size);

		void readBytesAtOffset(size_t offset, std::vector<uint8_t>& data) override;

		size_t getImageSize() override;
	};

    class VectorImageReader : public IImageReader
	{
		std::vector<uint8_t> m_data;
	public:
		VectorImageReader(const std::vector<uint8_t>& data);

		void readBytesAtOffset(size_t offset, std::vector<uint8_t>& data) override;

		size_t getImageSize() override;
	};

	class FileImageReader : public IImageReader, public ISerializable
	{
		std::filesystem::path m_pathToImgFile;
		std::unique_ptr<VectorImageReader> m_reader;
	public:
		static inline const std::string Name = "FileImageReader";

		FileImageReader(const std::filesystem::path& pathToImgFile = "");

		void readFile();

		void readBytesAtOffset(size_t offset, std::vector<uint8_t>& data) override;

		size_t getImageSize() override;

		void serialize(boost::json::object& data) const override;
		
		void deserialize(boost::json::object& data) override;
	};
};