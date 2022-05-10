#pragma once
#include "Object.h"

namespace sda
{
    class IFunction : public virtual IObject
    {
    public:
        virtual int64_t getOffset() const = 0;
    };

    class FunctionList : public ObjectList<IFunction>
    {
    };

    class Function : public Object, public IFunction
    {
        int64_t m_offset;
    public:
        Function() = default;

        Function(Context* context, int64_t offset);

        int64_t getOffset() const override;

        void bind(IContext* context) override;

        void serialize(boost::json::object& data) const override;

        void deserialize(boost::json::object& data) override;
    };
};