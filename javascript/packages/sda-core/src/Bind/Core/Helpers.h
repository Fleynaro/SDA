#pragma once
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/Platform/InstructionDecoder.h"
#include "SDA/Core/Factory.h"

namespace sda::bind
{
    class HelpersBind
    {
        static auto InstructionToV8(const Instruction* origInstr, Offset offset) {
            auto isolate = v8::Isolate::GetCurrent();
            std::list<v8::Local<v8::Object>> tokenObjs;
            for (auto& token : origInstr->tokens) {
                auto tokenObj = v8::Object::New(isolate);
                v8pp::set_option(isolate, tokenObj, "type", token.type);
                v8pp::set_option(isolate, tokenObj, "text", token.text);
                tokenObjs.push_back(tokenObj);
            }
            auto instrObj = v8::Object::New(isolate);
            v8pp::set_option(isolate, instrObj, "type", origInstr->type);
            v8pp::set_option(isolate, instrObj, "offset", offset);
            v8pp::set_option(isolate, instrObj, "length", origInstr->length);
            v8pp::set_option(isolate, instrObj, "targetOffset", offset + origInstr->jmpOffsetDelta);
            v8pp::set_option(isolate, instrObj, "tokens", tokenObjs);
            return instrObj;
        }

        static auto GetOriginalInstructions(Image* image, ImageSection* section) {
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
                result.push_back(InstructionToV8(origInstr, offset));
                offset += origInstr->length;
            }
            return result;
        }

        static auto GetOriginalInstructionInDetail(Image* image, Offset offset) {
            auto platform = image->getContext()->getPlatform();
            auto instrDecoder = platform->getInstructionDecoder();
            std::vector<uint8_t> buffer(100);
            image->getRW()->readBytesAtOffset(offset, buffer);
            instrDecoder->decode(buffer, true);
            auto origInstr = instrDecoder->getDecodedInstruction();
            return InstructionToV8(origInstr, offset);
        }

        static auto CreateContextObject(Context* context, boost::json::object& data) {
            return Factory(context).create(data);
        }
        
    public:
        static void Init(v8pp::module& module) {
            module.function("GetOriginalInstructions", &GetOriginalInstructions);
            module.function("GetOriginalInstructionInDetail", &GetOriginalInstructionInDetail);
            module.function("CreateContextObject", &CreateContextObject);
        }
    };
};