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

void Function::serialize(boost::json::object& data) const {
    Object::serialize(data);
    data["offset"] = m_offset;
}

void Function::deserialize(boost::json::object& data) {
    Object::deserialize(data);
    m_offset = data["offset"].get_int64();
}