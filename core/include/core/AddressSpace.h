#pragma once
#include "Object.h"
#include "ObjectList.h"

namespace sda
{
    class AddressSpace : public Object
    {
    public:
        AddressSpace(Context* context, ObjectId* id = nullptr);
    };


    class AddressSpaceList : public ObjectList<AddressSpace>
    {
    public:
        using ObjectList<AddressSpace>::ObjectList;
    };
};