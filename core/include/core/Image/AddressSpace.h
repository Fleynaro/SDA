#pragma once
#include <filesystem>
#include "Core/Object/ObjectList.h"

namespace sda
{
    class Image;

    class AddressSpace : public ContextObject
    {
        std::list<Image*> m_images;
    public:
        AddressSpace(Context* context, ObjectId* id = nullptr, const std::string& name = "");

        // Get the list of images in the address space
        const std::list<Image*>& getImages() const;

        // Get the image at the given address
        Image* getImageAt(std::uintptr_t address) const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
        
        static std::string GetCollectionName();
    };

    class AddressSpaceList : public ObjectList<AddressSpace>
    {
    public:
        using ObjectList<AddressSpace>::ObjectList;
    };
};