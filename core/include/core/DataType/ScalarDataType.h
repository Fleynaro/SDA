#pragma once
#include "Core/DataType/DataType.h"

namespace sda
{
    class ScalarDataType : public DataType
    {
        ScalarType m_scalarType;
        size_t m_size;
    public:
        static inline const std::string Type = "scalar";

        ScalarDataType(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            ScalarType scalarType = ScalarType::UnsignedInt,
            size_t size = 0
        );

        ScalarType getScalarType() const;

        bool isScalar(ScalarType type) const override;

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};