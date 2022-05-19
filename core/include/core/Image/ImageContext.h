#pragma once
#include "Core/Object/ObjectList.h"

namespace sda
{
    class SymbolTable;

    class ImageContext : public ContextObject
    {
        SymbolTable* m_globalSymbolTable;
        SymbolTable* m_funcBodySymbolTable;
    public:
        static inline const std::string Collection = "image_contexts";

        ImageContext(Context* context, Object::Id* id = nullptr, const std::string& name = "");

        SymbolTable* getGlobalSymbolTable() const;

        SymbolTable* getFuncBodySymbolTable() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
    };

    class ImageContextList : public ObjectList<ImageContext>
    {
    public:
        using ObjectList<ImageContext>::ObjectList;
    };
};