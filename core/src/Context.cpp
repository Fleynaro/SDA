#include "Core/Context.h"
#include "Core/Function.h"

using namespace sda;

Context::Context() {
    m_functions = std::make_unique<FunctionList>();
}

std::string Context::getName() const {
    return "core";
}

FunctionList* Context::getFunctions() {
    return m_functions.get();
}