#pragma once
#include "Core/DataType/DataType.h"

namespace sda
{
    class TypedefDataType : public DataType
    {
        DataType* m_refType;
    public:
        static inline const std::string Type = "typedef";

        TypedefDataType(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* refType = nullptr);

        void setReferenceType(DataType* refType);

        DataType* getReferenceType() const;

        DataType* getBaseType() override;

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};