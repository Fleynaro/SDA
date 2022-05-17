#pragma once
#include <string>
#include <vector>

namespace sda
{
    // Image reader interface
    class IImageReader
	{
	public:
        // Read bytes from the image at the given offset
		virtual void readBytesAtOffset(uint64_t offset, std::vector<uint8_t>& bytes) = 0;

        // Get the size of the image
		virtual size_t getImageSize() = 0;
	};

    class PointerImageReader : public IImageReader
	{
        uint8_t* m_data;
		size_t m_size;
	public:
		PointerImageReader(uint8_t* data, size_t size);

		void readBytesAtOffset(uint64_t offset, std::vector<uint8_t>& data) override;

		size_t getImageSize() override;
	};

    class VectorImageReader : public IImageReader
	{
		std::vector<uint8_t> m_data;
	public:
		VectorImageReader(const std::vector<uint8_t>& data);

		void readBytesAtOffset(uint64_t offset, std::vector<uint8_t>& data) override;

		size_t getImageSize() override;
	};
};