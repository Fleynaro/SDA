#pragma once
#include "SDA/Core/DataType/DataType.h"

namespace sda
{
    class VoidDataType : public DataType
    {
    public:
        static inline const std::string Type = "void";

        VoidDataType(Context* context, Object::Id* id = nullptr);

        bool isVoid() const override;

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};