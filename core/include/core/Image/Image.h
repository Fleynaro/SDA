#pragma once
#include "Core/Object/ObjectList.h"
#include "ImageReader.h"
#include "ImageAnalyser.h"

namespace sda
{
    class Image : public ContextObject
    {
        std::unique_ptr<IImageReader> m_reader;
        std::shared_ptr<IImageAnalyser> m_analyser;

    public:
        Image(
            Context* context,
            std::unique_ptr<IImageReader> reader,
            std::shared_ptr<IImageAnalyser> analyser,
            ObjectId* id = nullptr,
            const std::string& name = "");
        
        void analyse();

        bool contains(std::uintptr_t address) const;

        Image* clone(std::unique_ptr<IImageReader> reader) const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;

        static std::string GetCollectionName();
    };

    class ImageList : public ObjectList<Image>
    {
    public:
        using ObjectList<Image>::ObjectList;
    };
};