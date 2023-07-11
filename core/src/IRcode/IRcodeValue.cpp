#include "SDA/Core/IRcode/IRcodeValue.h"
#include "SDA/Core/IRcode/IRcodeOperation.h"
#include "SDA/Core/IRcode/IRcodeFunction.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/StructureDataType.h"

using namespace sda;
using namespace sda::ircode;

Value::Value(Hash hash)
    : m_hash(hash)
{}

Hash Value::getHash() const {
    return m_hash;
}

std::list<Operation*> Value::getOperations() const {
    return m_operations;
}

void Value::addOperation(Operation* operation) {
    m_operations.push_back(operation);
}

void Value::removeOperation(Operation* operation) {
    m_operations.remove(operation);
}

LinearExpression Value::GetLinearExpr(std::shared_ptr<Value> value, bool goThroughRef) {
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

Constant::Constant(std::shared_ptr<pcode::ConstantVarnode> constVarnode, Hash hash)
    : Value(hash), m_constVarnode(constVarnode)
{}

Value::Type Constant::getType() const {
    return Type::Constant;
}

std::shared_ptr<pcode::ConstantVarnode> Constant::getConstVarnode() const {
    return m_constVarnode;
}

size_t Constant::getSize() const {
    return m_constVarnode->getSize();
}

ircode::Register::Register(std::shared_ptr<pcode::RegisterVarnode> regVarnode, Hash hash)
    : Value(hash), m_regVarnode(regVarnode)
{}

Value::Type ircode::Register::getType() const {
    return Type::Register;
}

std::shared_ptr<pcode::RegisterVarnode> ircode::Register::getRegVarnode() const {
    return m_regVarnode;
}

const sda::Register& ircode::Register::getRegister() const {
    return m_regVarnode->getRegister();
}

size_t ircode::Register::getSize() const {
    return m_regVarnode->getSize();
}

Variable::Variable(size_t id, const MemoryAddress& memAddress, Hash hash, size_t size)
    : Value(hash)
    , m_id(id)
    , m_memAddress(memAddress)
    , m_size(size)
{}

size_t Variable::getId() const {
    return m_id;
}

std::string Variable::getName(bool full) const {
    if (full) {
        if (auto varFunction = getSourceOperation()->getBlock()->getFunction()) {
            return varFunction->getName() + ":" + getName(false);
        }
    }
    return std::string("var") + std::to_string(m_id);
}

Value::Type Variable::getType() const {
    return Type::Variable;
}

Operation* Variable::getSourceOperation() const {
    return m_operations.empty() ? nullptr : m_operations.front();
}

std::list<RefOperation*> Variable::getRefOperations() const {
    std::list<RefOperation*> refOperations;
    auto srcOp = getSourceOperation();
    for (auto operation : m_operations) {
        // ignore source operation
        if (operation == srcOp) {
            continue;
        }
        if (auto refOperation = dynamic_cast<RefOperation*>(operation)) {
            refOperations.push_back(refOperation);
        }
    }
    return refOperations;
}

bool Variable::isUsed() const {
    return m_operations.size() > 1;
}

const MemoryAddress& Variable::getMemAddress() const {
    return m_memAddress;
}

size_t Variable::getSize() const {
    return m_size;
}
