#pragma once
#include "Core/DataType/DataType.h"
#include "Core/Symbol/StructureFieldSymbol.h"
#include "Core/SymbolTable/StandartSymbolTable.h"

namespace sda
{
    class StructureDataType : public DataType
    {
        size_t m_size;
        StandartSymbolTable* m_symbolTable;
    public:
        static inline const std::string Type = "structure";

        StructureDataType(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            bool symbolTable = false);

        StandartSymbolTable* getSymbolTable();

        void setSize(size_t size);

        size_t getSize() const override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};