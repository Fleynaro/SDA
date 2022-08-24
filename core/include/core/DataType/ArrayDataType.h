#pragma once
#include "Core/DataType/DataType.h"

namespace sda
{
    class ArrayDataType : public DataType
    {
        DataType* m_elementType;
        std::list<size_t> m_dimensions;
    public:
        static inline const std::string Type = "array";

        ArrayDataType(
            Context* context,
            Object::Id* id = nullptr,
            DataType* elementType = nullptr,
            const std::list<size_t>& dimensions = std::list<size_t>());

        DataType* getElementType() const;

        const std::list<size_t>& getDimensions() const;

        DataType* getBaseType() override;

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        static std::string GetTypeName(DataType* elementType, const std::list<size_t>& dimensions);
    };
};