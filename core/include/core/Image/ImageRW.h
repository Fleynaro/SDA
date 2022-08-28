#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "Core/Utils/Serialization.h"
#include "Core/Offset.h"

namespace sda
{
    // Image reader-writer interface
    class IImageRW
	{
	public:
        // Read bytes from the image at the given offset
		virtual void readBytesAtOffset(Offset offset, std::vector<uint8_t>& bytes) = 0;

		// Write bytes to the image at the given offset
		virtual void writeBytesAtOffset(Offset offset, const std::vector<uint8_t>& bytes) = 0;

        // Get the size of the image
		virtual size_t getImageSize() = 0;
	};

    class PointerImageRW : public IImageRW
	{
        uint8_t* m_data;
		size_t m_size;
	public:
		PointerImageRW(uint8_t* data, size_t size);

		void readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) override;

		void writeBytesAtOffset(Offset offset, const std::vector<uint8_t>& data) override;

		size_t getImageSize() override;
	};

    class VectorImageRW : public IImageRW
	{
		std::vector<uint8_t> m_data;
	public:
		VectorImageRW(const std::vector<uint8_t>& data);

		void readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) override;

		void writeBytesAtOffset(Offset offset, const std::vector<uint8_t>& data) override;

		size_t getImageSize() override;

		const std::vector<uint8_t>& getData() const;
	};

	class FileImageRW : public IImageRW, public utils::ISerializable
	{
		std::filesystem::path m_pathToImgFile;
		std::unique_ptr<VectorImageRW> m_rw;
	public:
		static inline const std::string Name = "FileImageRW";

		FileImageRW(const std::filesystem::path& pathToImgFile = "");

		void readFile();

		void saveFile();

		void readBytesAtOffset(Offset offset, std::vector<uint8_t>& data) override;

		void writeBytesAtOffset(Offset offset, const std::vector<uint8_t>& data) override;

		size_t getImageSize() override;

		void serialize(boost::json::object& data) const override;
		
		void deserialize(boost::json::object& data) override;
	};
};