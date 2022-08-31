#pragma once
#include "Core/DataType/DataType.h"
#include <v8pp/class.hpp>
#include <v8pp/module.hpp>

namespace sda::bind
{
    class DataTypeBind
    {
    public:
        DataType* m_instance;

        DataTypeBind() = default;

        DataTypeBind(DataType* instance)
            : m_instance(instance)
        {}

        bool isVoid() const {
            return m_instance->isVoid();
        }

        size_t getSize() const {
            return m_instance->getSize();
        }

        static void Init(v8pp::module& module) {
            v8pp::class_<DataTypeBind> cl(module.isolate());
            cl.property("isVoid", &DataTypeBind::isVoid);
            cl.property("size", &DataTypeBind::getSize);
            module.class_("DataType", cl);
        }
    };
};