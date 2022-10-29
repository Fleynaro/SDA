#pragma once
#include "SDA/Core/Utils/Serialization.h"
#include "SDA/Core/Utils/AbstractParser.h"
#include "SDA/Core/Utils/AbstractPrinter.h"

namespace sda::bind
{
    class SerializationBind
    {
        static auto Serialize(utils::ISerializable* obj) {
            boost::json::object data;
            obj->serialize(data);
            auto isolate = v8::Isolate::GetCurrent();
            return v8pp::json_parse(isolate, boost::json::serialize(data));
        }

        static void Deserialize(utils::ISerializable* obj, v8::Local<v8::Value> value) {
            auto isolate = v8::Isolate::GetCurrent();
            if (!value->IsObject()) {
                v8pp::throw_ex(isolate, "Invalid argument: expected object");
                return;
            }
            auto json = v8pp::json_str(isolate, value);
            boost::json::object data = boost::json::parse(json).as_object();
            obj->deserialize(data);
        }
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<utils::ISerializable>(module);
            cl
                .method("serialize", &Serialize)
                .method("deserialize", &Deserialize);
            module.class_("Serialization", cl);
        }
    };

    class AbstractPrinterBind
    {
    protected:
        template<typename T, typename... Args>
        class PrinterJs : public T {
            std::stringstream ss;

            using T::T;
            
            static auto New(Args... args) {
                auto printer = new PrinterJs(std::move(args)...);
                printer->setOutput(printer->ss);
                return ExportObject(printer);
            }

        public:
            static auto Create(v8pp::module& module) {
                auto cl = NewClass<PrinterJs>(module);
                cl
                    .inherit<utils::AbstractPrinter>()
                    .property("output", [](PrinterJs& self) { return self.ss.str(); })
                    .method("flush", std::function([](PrinterJs* self) { self->ss.str(""); }))
                    .static_method("New", &New);
                return cl;
            }
        };
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<utils::AbstractPrinter>(module);
            cl
                .method("setTabSize", &utils::AbstractPrinter::setTabSize)
                .method("setParentPrinter", &utils::AbstractPrinter::setParentPrinter)
                .method("startBlock", &utils::AbstractPrinter::startBlock)
                .method("endBlock", &utils::AbstractPrinter::endBlock)
                .method("startCommenting", &utils::AbstractPrinter::startCommenting)
                .method("endCommenting", &utils::AbstractPrinter::endCommenting)
                .method("newLine", &utils::AbstractPrinter::newLine);
            module.class_("AbstractPrinter", cl);
        }
    };
};