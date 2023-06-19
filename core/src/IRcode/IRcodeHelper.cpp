#include "SDA/Core/IRcode/IRcodeHelper.h"

using namespace sda;
using namespace sda::ircode;

const sda::Register* ircode::ExtractRegister(std::shared_ptr<ircode::Value> value)
{
    if (auto variable = std::dynamic_pointer_cast<ircode::Variable>(value)) {
        if (auto unarySrcOp = dynamic_cast<const ircode::UnaryOperation*>(variable->getSourceOperation())) {
            if (unarySrcOp->getId() == ircode::OperationId::LOAD) {
                if (auto reg = std::dynamic_pointer_cast<ircode::Register>(unarySrcOp->getInput())) {
                    return &reg->getRegister();
                }
            }
        }
    }
    return nullptr;
}