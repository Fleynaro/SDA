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
    {};

    class Function : public Object, public IFunction
    {
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::base_object<Object>(*this);
            ar & m_offset;
        }

        int64_t m_offset;
    public:
        Function() = default;

        int64_t getOffset() const override;

        static Function* Create(Context* context, int64_t offset);
    };
};