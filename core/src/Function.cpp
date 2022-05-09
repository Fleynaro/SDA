#include "Core/Function.h"
#include "Core/Context.h"

using namespace sda;

int64_t Function::getOffset() const {
    return m_offset;
}

Function* Function::Create(Context* context, int64_t offset) {
    auto function = new Function();
    function->m_offset = offset;
    
    context->getFunctions()->add(std::unique_ptr<Function>(function));

    return function;
}