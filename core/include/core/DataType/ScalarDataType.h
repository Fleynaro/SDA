#pragma once
#include "Core/DataType/DataType.h"

namespace sda
{
    class ScalarDataType : public DataType
    {
        bool m_isFloatingPoint;
        bool m_isSigned;
        size_t m_size;
    public:
        static inline const std::string Type = "scalar";

        ScalarDataType(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            bool isFloatingPoint = false,
            bool isSigned = false,
            size_t size = 0
        );

        bool isFloatingPoint() const;

        bool isSigned() const;

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};