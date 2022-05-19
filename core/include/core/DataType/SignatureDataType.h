#pragma once
#include "Core/DataType/DataType.h"

namespace sda
{
    class SignatureDataType : public DataType
    {
    public:
        static inline const std::string Type = "signature";

        SignatureDataType(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};