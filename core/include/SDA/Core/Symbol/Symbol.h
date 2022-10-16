#pragma once
#include "SDA/Core/Object/ObjectList.h"

namespace sda
{
    class DataType;

    class Symbol : public ContextObject
    {
        DataType* m_dataType;
    protected:
        Symbol(
            Context* context,
            Object::Id* id = nullptr,
            const std::string& name = "",
            DataType* dataType = nullptr);
            
    public:
        static inline const std::string Collection = "symbols";

        DataType* getDataType() const;

        void setDataType(DataType* dataType);

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