#pragma once
#include "Core/DataType/DataType.h"
#include "Core/Symbol/StructureFieldSymbol.h"

namespace sda
{
    class StructureDataType : public DataType
    {
        std::map<Offset, StructureFieldSymbol*> m_fields;
    public:
        static inline const std::string Type = "structure";

        StructureDataType(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        void addField(StructureFieldSymbol* field);

        void removeField(StructureFieldSymbol* field);

        const std::map<Offset, StructureFieldSymbol*>& getFields() const;

        StructureFieldSymbol* getFieldAt(Offset offset) const;

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};