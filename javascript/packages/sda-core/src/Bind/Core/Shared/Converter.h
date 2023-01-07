#pragma once
#include "SDA/Core/Pcode/PcodeInstruction.h"

namespace v8pp
{
    // for sda::pcode::InstructionOffset
    template<>
    struct convert<sda::pcode::InstructionOffset>
    {
        using underlying_type = sda::Offset;

        using from_type = sda::pcode::InstructionOffset;
        using to_type = typename convert<underlying_type>::to_type;

        static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value) {
            return convert<underlying_type>::is_valid(isolate, value);
        }

        static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value) {
            if (!is_valid(isolate, value)) {
                throw std::invalid_argument("expected number for InstructionOffset");
            }
            auto offset = convert<underlying_type>::from_v8(isolate, value);
            return sda::pcode::InstructionOffset(offset);
        }

        static to_type to_v8(v8::Isolate* isolate, const from_type& value) {
            return convert<underlying_type>::to_v8(isolate, value);
        }
    };
};