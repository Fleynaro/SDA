#include "Core/Function.h"
#include "Core/Context.h"

using namespace sda;

int64_t Function::getOffset() const {
    return m_offset;
}

Function* Function::Create(Context* context, int64_t offset) {
    auto function = std::make_unique<Function>();
    auto pFunction = function.get();

    function->m_offset = offset;
    context->getFunctions()->add(std::move(function));

    return pFunction;
}