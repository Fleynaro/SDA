#pragma once
#include "Core/DataType/DataType.h"
#include "Core/Symbol/StructureFieldSymbol.h"

namespace sda
{
    class StructureDataType : public DataType
    {
        size_t m_size;
        std::map<Offset, StructureFieldSymbol*> m_fields;
    public:
        static inline const std::string Type = "structure";

        StructureDataType(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        // todo: to change the fields, use setFields() only (like in react js)
        void setFields(const std::map<Offset, StructureFieldSymbol*>& fields);

        const std::map<Offset, StructureFieldSymbol*>& getFields() const;

        void setSize(size_t size);

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};