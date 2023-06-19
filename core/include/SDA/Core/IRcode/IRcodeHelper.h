#pragma once
#include "SDA/Core/IRcode/IRcodeOperation.h"

namespace sda::ircode
{
    const sda::Register* ExtractRegister(std::shared_ptr<ircode::Value> value);
};
