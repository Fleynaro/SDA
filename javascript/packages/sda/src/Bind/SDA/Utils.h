#pragma once
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/Platform/InstructionDecoder.h"

namespace sda::bind
{
    class UtilsBind
    {
        static auto GetOriginalInstructions(Image* image, ImageSection* section) {
            auto isolate = v8::Isolate::GetCurrent();
            std::list<v8::Local<v8::Object>> result;
            auto platform = image->getContext()->getPlatform();
            auto instrDecoder = platform->getInstructionDecoder();
            auto offset = section->getMinOffset();
            while (offset < section->getMaxOffset())
            {
                std::vector<uint8_t> buffer(100);
                image->getRW()->readBytesAtOffset(offset, buffer);
                instrDecoder->decode(buffer, false);
                auto origInstr = instrDecoder->getDecodedInstruction();
                auto targetOffset = offset + origInstr->jmpOffsetDelta;
                auto instrObj = v8::Object::New(isolate);
                v8pp::set_option(isolate, instrObj, "type", origInstr->type);
                v8pp::set_option(isolate, instrObj, "offset", offset);
                v8pp::set_option(isolate, instrObj, "length", origInstr->length);
                v8pp::set_option(isolate, instrObj, "targetOffset", targetOffset);
                result.push_back(instrObj);
                offset += origInstr->length;
            }
            return result;
        }
        
    public:
        static void Init(v8pp::module& module) {
            // auto cl = NewClass<UtilsBind>(module);
            // cl
            //     .static_method("GetOriginalInstructions", &GetOriginalInstructions);
            // module.class_("Utils", cl);
            module.function("GetOriginalInstructions", &GetOriginalInstructions);
        }
    };
};