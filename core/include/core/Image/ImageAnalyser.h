#pragma once
#include "Core/Serialization.h"
#include "Core/Image/WinImageHeaders.h"

namespace sda
{
    class Image;

    class IImageAnalyser
    {
    public:
        virtual void analyse(Image* image) = 0;
    };

    class PEImageAnalyser : public IImageAnalyser, public ISerializable
    {
        Image* m_image = nullptr;
        std::unique_ptr<windows::__IMAGE_DOS_HEADER> m_imgDosHeader;
        std::unique_ptr<windows::__IMAGE_NT_HEADERS> m_imgNtHeaders;
    public:
        void analyse(Image* image) override;

    private:
        void analyseHeaders();

		void analyseSections();

    public:
        void serialize(boost::json::object& data) const override;
    };
};