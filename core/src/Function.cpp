#include "Core/Function.h"
#include "Core/Context.h"

using namespace sda;

Function::Function(Context* context, int64_t offset)
    : m_offset(offset)
{
    bind(context);
}

int64_t Function::getOffset() const {
    return m_offset;
}

void Function::bind(IContext* context) {
    if(auto ctx = dynamic_cast<Context*>(context)) {
        ctx->getFunctions()->add(std::unique_ptr<Function>(this));
    }
}