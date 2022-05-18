#pragma once
#include "Core/Object/ObjectList.h"

namespace sda
{
    class DataType : public ContextObject
    {
    public:
        static inline const std::string CollectionName = "data_types";

        DataType(Context* context, ObjectId* id = nullptr, const std::string& name = "");

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
    };

    class DataTypeList : public ObjectList<DataType>
    {
    public:
        using ObjectList<DataType>::ObjectList;
    };
};