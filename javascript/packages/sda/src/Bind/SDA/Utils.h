#pragma once
#include "SDA/Core/Image/Image.h"

namespace sda::bind
{
    class UtilsBind
    {
        static auto GetOriginalInstructions(Image* image, ImageSection* section) {
            auto isolate = v8::Isolate::GetCurrent();
            std::map<Offset, v8::Local<v8::Object>> result;
            auto platform = image->getContext()->getPlatform();
            auto instrDecoder = platform->getInstructionDecoder();
            auto offset = section->getMinOffset();
			while (offset < section->getMaxOffset())
			{
                std::vector<uint8_t> buffer(100);
                image->getRW()->readBytesAtOffset(offset, buffer);
                instrDecoder->decode(buffer);
                auto origInstr = instrDecoder->getDecodedInstruction();
                auto targetOffset = offset + origInstr->jmpOffsetDelta;
                auto instrObj = v8::Object::New(isolate);
                v8pp::set_option(isolate, instrObj, "type", origInstr->type);
                v8pp::set_option(isolate, instrObj, "length", origInstr->length);
                v8pp::set_option(isolate, instrObj, "targetOffset", targetOffset);
                result[offset] = instrObj;
            }
            // https://github.com/Fleynaro/SDA/blob/d4b977e9522f09da38847a8e95b7f471e3a30f14/src/gui/controllers/ImageContentController.h#L238
            return result;
        }
        
    public:
        static void Init(v8pp::module& module) {
            auto cl = NewClass<UtilsBind>(module);
            cl
                .static_method("New", &New);
            module.class_("Utils", cl);
        }
    };
};