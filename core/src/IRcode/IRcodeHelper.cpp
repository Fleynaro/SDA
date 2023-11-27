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

LinearExpression ircode::GetLinearExpr(std::shared_ptr<Value> value, bool goThroughRef) {
    if (auto var = std::dynamic_pointer_cast<ircode::Variable>(value)) {
        auto srcOp = var->getSourceOperation();
        if (auto unaryOp = dynamic_cast<const ircode::UnaryOperation*>(srcOp)) {
            if (unaryOp->getId() == ircode::OperationId::COPY || (goThroughRef && unaryOp->getId() == ircode::OperationId::REF)) {
                return GetLinearExpr(unaryOp->getInput(), goThroughRef);
            }
        }
        else if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(srcOp)) {
            if (binaryOp->getId() == ircode::OperationId::INT_ADD || binaryOp->getId() == ircode::OperationId::INT_MULT) {
                auto linearExprInp1 = GetLinearExpr(binaryOp->getInput1(), goThroughRef);
                auto linearExprInp2 = GetLinearExpr(binaryOp->getInput2(), goThroughRef);
                if (binaryOp->getId() == ircode::OperationId::INT_ADD) {
                    return linearExprInp1 + linearExprInp2;
                }
                else {
                    if (linearExprInp1.getTerms().empty() || linearExprInp2.getTerms().empty()) {
                        return linearExprInp1 * linearExprInp2;
                    }
                }
            }
        }
    }
    else if (auto reg = std::dynamic_pointer_cast<ircode::Register>(value)) {
        auto offset = reg->getRegister().getBitOffset() / utils::BitsInBytes;
        return LinearExpression(value) + LinearExpression(offset);
    }
    else if (auto constant = std::dynamic_pointer_cast<ircode::Constant>(value)) {
        return LinearExpression(constant->getConstVarnode()->getValue());
    }
    return LinearExpression(value);
}

std::list<std::shared_ptr<Value>> ircode::ToBaseTerms(const LinearExpression& linearExpr, Platform* platform) {
    std::list<std::shared_ptr<Value>> baseTerms;
    for (auto& term : linearExpr.getTerms()) {
        if (term.factor != 1 || term.value->getSize() != platform->getPointerSize())
            continue;
        baseTerms.push_back(term.value);
    }
    return baseTerms;
}