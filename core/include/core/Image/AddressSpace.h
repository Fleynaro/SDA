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
        static inline const std::string Collection = "address_spaces";

        AddressSpace(Context* context, ObjectId* id = nullptr, const std::string& name = "");

        // Add an image to the address space
        void addImage(Image* image);

        // Get the list of images in the address space
        const std::list<Image*>& getImages() const;

        // Get the image at the given address
        Image* getImageAt(std::uintptr_t address) const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
    };

    class AddressSpaceList : public ObjectList<AddressSpace>
    {
    public:
        using ObjectList<AddressSpace>::ObjectList;
    };
};