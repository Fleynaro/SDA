#pragma once

namespace sda
{
    class Image;

    class IImageAnalyser
    {
    public:
        virtual void analyse(Image* image) = 0;
    };
};