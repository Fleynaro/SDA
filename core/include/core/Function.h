#pragma once
#include "Object.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

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
            ar & m_offset;
        }

        int64_t m_offset;
    public:
        Function() = default;

        int64_t getOffset() const override;

        void serialize(JsonData& data) const override {
            Object::serialize(data);
            //data["offset"] = m_offset;
        }

        void deserialize(const JsonData& data) override {
            Object::deserialize(data);
            //m_offset = data["offset"];
        }

        static Function* Create(Context* context, int64_t offset);
    };
};