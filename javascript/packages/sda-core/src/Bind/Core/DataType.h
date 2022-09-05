#pragma once
#include "Core/DataType/VoidDataType.h"

namespace sda::bind
{
    class DataTypeBind
    {
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<DataType> cl(module.isolate());
            cl.inherit<ContextObject>();
            cl.property("isVoid", &DataType::isVoid);
            cl.property("size", &DataType::getSize);
            cl.property("getType", [](const DataType& self) {
                return new VoidDataType(new Context(nullptr));
            });
            module.class_("DataType", cl);
        }
    };

    class VoidDataTypeBind : public DataTypeBind
    {
        static auto Create(Context* ctx) {
            return CreateContextObject(ctx, [ctx]() {
                auto dataType = new VoidDataType(ctx);
                return ExportObjectRef(dataType);
            });
        }
    public:
        static void Init(v8pp::module& module) {
            v8pp::class_<VoidDataType> cl(module.isolate());
            cl.inherit<DataType>();
            cl.function("create", &Create);
            module.class_("VoidDataType", cl);
        }
    };
};