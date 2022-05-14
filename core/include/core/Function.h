#pragma once
#include "Object.h"
#include "ObjectList.h"

namespace sda
{
    class IFunction : public virtual IObject
    {
    public:
        virtual int64_t getOffset() const = 0;
    };

    class FunctionList : public ObjectList<IFunction>
    {
    public:
        using ObjectList<IFunction>::ObjectList;
    };

    class Function : public Object, public IFunction
    {
        Context* m_context;
        int64_t m_offset;
    public:
        Function(Context* context, ObjectId* id = nullptr, int64_t offset = 0);

        int64_t getOffset() const override;
        
        Function* clone() const;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};