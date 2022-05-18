#pragma once
#include "Core/Object/ObjectList.h"

namespace sda
{
    class DataType;

    class Symbol : public ContextObject
    {
        DataType* m_dataType;
    public:
        static inline const std::string Collection = "symbols";

        Symbol(
            Context* context,
            ObjectId* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr);

        DataType* getDataType() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
    };

    class SymbolList : public ObjectList<Symbol>
    {
    public:
        using ObjectList<Symbol>::ObjectList;
    };
};