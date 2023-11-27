#pragma once
#include "SDA/Core/IRcode/IRcodeOperation.h"

namespace sda::ircode
{
    const sda::Register* ExtractRegister(std::shared_ptr<ircode::Value> value);

    bool ExtractConstant(std::shared_ptr<ircode::Value> value, size_t& constValue);

    std::shared_ptr<Value> ExtractAddressValue(std::shared_ptr<ircode::Value> value);

    LinearExpression GetLinearExpr(std::shared_ptr<Value> value, bool goThroughRef = false);

    std::list<std::shared_ptr<Value>> ToBaseTerms(const LinearExpression& linearExpr, Platform* platform);
};
