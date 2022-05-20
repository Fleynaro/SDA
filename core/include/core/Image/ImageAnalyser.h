#pragma once
#include "ImageSection.h"
#include "Core/Serialization.h"
#include "Core/Image/WinImageHeaders.h"

namespace sda
{
    class Image;

    class ImageAnalyser
    {
    public:
        std::uintptr_t m_baseAddress = 0;
        size_t m_entryPointOffset = 0;
        std::list<ImageSection> m_imageSections;

        virtual void analyse(Image* image) = 0;
    };

    class PEImageAnalyser : public ImageAnalyser, public ISerializable
    {
        Image* m_image = nullptr;
        std::unique_ptr<windows::__IMAGE_DOS_HEADER> m_imgDosHeader;
        std::unique_ptr<windows::__IMAGE_NT_HEADERS> m_imgNtHeaders;
    public:
        static inline const std::string Name = "PEImageAnalyser";

        void analyse(Image* image) override;

    private:
        void analyseHeaders();

		void analyseSections();

    public:
        void serialize(boost::json::object& data) const override;
    };
};