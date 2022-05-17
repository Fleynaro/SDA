#pragma once
#include "Object.h"
#include "ObjectList.h"

namespace sda
{
    class Function : public ContextObject
    {
        int64_t m_offset;
    public:
        Function(Context* context, ObjectId* id = nullptr, const std::string& name = "", int64_t offset = 0);

        int64_t getOffset() const;

        void setOffset(int64_t offset);
        
        Function* clone() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;

        void destroy() override;
    };

    class FunctionList : public ObjectList<Function>
    {
    public:
        using ObjectList<Function>::ObjectList;
    };
};