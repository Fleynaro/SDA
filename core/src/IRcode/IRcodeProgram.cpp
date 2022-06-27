#include "Core/IRcode/IRcodeProgram.h"

using namespace sda;
using namespace sda::ircode;

std::map<pcode::FunctionGraph*, Function>& Program::getFunctions() {
    return m_functions;
}