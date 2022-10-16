#pragma once
#include "ImageSection.h"
#include "SDA/Core/Utils/Serialization.h"
#include "SDA/Core/Image/WinImageHeaders.h"

namespace sda
{
    class IImageRW;

    class ImageAnalyser
    {
    public:
        std::uintptr_t m_baseAddress = 0;
        Offset m_entryPointOffset = 0;
        std::list<ImageSection> m_imageSections;

        virtual void analyse(IImageRW* rw) = 0;
    };

    class TestAnalyser : public ImageAnalyser
    {
    public:
        void analyse(IImageRW* rw) override;
    };

    class PEImageAnalyser : public ImageAnalyser, public utils::ISerializable
    {
        IImageRW* m_rw = nullptr;
        std::unique_ptr<windows::__IMAGE_DOS_HEADER> m_imgDosHeader;
        std::unique_ptr<windows::__IMAGE_NT_HEADERS> m_imgNtHeaders;
    public:
        static inline const std::string Name = "PEImageAnalyser";

        void analyse(IImageRW* rw) override;

    private:
        void analyseHeaders();

		void analyseSections();

    public:
        void serialize(boost::json::object& data) const override;
    };
};