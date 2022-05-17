#pragma once
#include "Core/Object/ObjectList.h"


namespace sda
{
    class Symbol : public ContextObject
    {
    public:
        Symbol(Context* context, ObjectId* id = nullptr, const std::string& name = "");

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;

        static std::string GetCollectionName();
    };

    class SymbolList : public ObjectList<Symbol>
    {
    public:
        using ObjectList<Symbol>::ObjectList;
    };
};