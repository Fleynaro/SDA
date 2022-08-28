#pragma once
#include <gmock/gmock.h>
#include "Core/Image/ImageRW.h"

namespace sda::test
{
    class ImageRWMock : public IImageRW
    {
    public:
        MOCK_METHOD(void, readBytesAtOffset, (Offset offset, std::vector<uint8_t>& bytes), (override));

        MOCK_METHOD(void, writeBytesAtOffset, (Offset offset, const std::vector<uint8_t>& bytes), (override));

        MOCK_METHOD(size_t, getImageSize, (), (override));
    };
};