#include "Core/Context.h"
#include "Core/Function.h"

using namespace sda;

Context::Context()
{
    m_functions = std::make_unique<FunctionList>(this);
    m_callbacks = std::make_unique<Callbacks>();
}

FunctionList* Context::getFunctions() {
    return m_functions.get();
}

std::unique_ptr<Context::Callbacks> Context::setCallbacks(std::unique_ptr<Callbacks> callbacks) {
    auto oldCallbacks = std::move(m_callbacks);
    m_callbacks = std::move(callbacks);
    return oldCallbacks;
}

Context::Callbacks* Context::getCallbacks() {
    return m_callbacks.get();
}