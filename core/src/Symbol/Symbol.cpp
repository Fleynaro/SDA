#include "Core/Symbol/Symbol.h"

using namespace sda;

Symbol::Symbol(Context* context, ObjectId* id, const std::string& name)
    : ContextObject(context, id, name)
{
    m_context->getSymbols()->add(std::unique_ptr<Symbol>(this));
}

void Symbol::serialize(boost::json::object& data) const {
    ContextObject::serialize(data);
}

void Symbol::deserialize(boost::json::object& data) {
    ContextObject::deserialize(data);
}

void Symbol::destroy() {
    m_context->getSymbols()->remove(getId());
}