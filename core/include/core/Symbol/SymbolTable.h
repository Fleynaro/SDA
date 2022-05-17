#pragma once
#include "Core/Object/ObjectList.h"

namespace sda
{
    class SymbolTable : public ContextObject
    {
    public:
        SymbolTable(Context* context, ObjectId* id = nullptr, const std::string& name = "");

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;

        static std::string GetCollectionName();
    };

    class SymbolTableList : public ObjectList<SymbolTable>
    {
    public:
        using ObjectList<SymbolTable>::ObjectList;
    };
};