#pragma once
#include <gmock/gmock.h>
#include "Core/Image/ImageAnalyser.h"

namespace sda::test
{
    class ImageAnalyserMock : public ImageAnalyser
    {
    public:
        MOCK_METHOD(void, analyse, (IImageRW* rw), (override));
    };
};