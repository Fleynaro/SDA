#pragma once
#include "ImageSection.h"
#include "Core/Serialization.h"
#include "Core/Image/WinImageHeaders.h"

namespace sda
{
    class IImageReader;

    class ImageAnalyser
    {
    public:
        std::uintptr_t m_baseAddress = 0;
        Offset m_entryPointOffset = 0;
        std::list<ImageSection> m_imageSections;

        virtual void analyse(IImageReader* reader) = 0;
    };

    class TestAnalyser : public ImageAnalyser
    {
    public:
        void analyse(IImageReader* reader) override;
    };

    class PEImageAnalyser : public ImageAnalyser, public ISerializable
    {
        IImageReader* m_reader = nullptr;
        std::unique_ptr<windows::__IMAGE_DOS_HEADER> m_imgDosHeader;
        std::unique_ptr<windows::__IMAGE_NT_HEADERS> m_imgNtHeaders;
    public:
        static inline const std::string Name = "PEImageAnalyser";

        void analyse(IImageReader* reader) override;

    private:
        void analyseHeaders();

		void analyseSections();

    public:
        void serialize(boost::json::object& data) const override;
    };
};