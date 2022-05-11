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

void Context::setCallbacks(std::unique_ptr<Callbacks> callbacks) {
    m_callbacks = std::move(callbacks);
}

Context::Callbacks* Context::getCallbacks() {
    return m_callbacks.get();
}