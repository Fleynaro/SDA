#include "Core/IRcode/IRcodeValue.h"

using namespace sda;
using namespace sda::ircode;

std::list<Operation*>& Value::getOperations() {
    return m_operations;
}

pcode::ConstantVarnode* Constant::getConstVarnode() const {
    return m_constVarnode;
}

pcode::RegisterVarnode* Register::getRegVarnode() const {
    return m_regVarnode;
}

Value* Variable::getAddressValue() const {
    return m_addressValue;
}