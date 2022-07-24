#include "Decompiler/IRcodeGenerator/IRcodeBlockGenerator.h"
#include <boost/functional/hash.hpp>

using namespace sda;
using namespace sda::decompiler;

MemorySpace* TotalMemorySpace::getMemSpace(ircode::Hash baseAddrHash) {
    auto it = m_memorySpaces.find(baseAddrHash);
    if (it == m_memorySpaces.end()) {
        m_memorySpaces[baseAddrHash] = MemorySpace();
        return &m_memorySpaces[baseAddrHash];
    }
    return &it->second;
}

IRcodeBlockGenerator::IRcodeBlockGenerator(ircode::Block* block, TotalMemorySpace* totalMemSpace)
    : m_block(block), m_totalMemSpace(totalMemSpace)
{}

const std::map<pcode::InstructionId, ircode::OperationId> InstructionToOperation = {
    // Arithmetic
    { pcode::InstructionId::INT_ADD, ircode::OperationId::INT_ADD },
    { pcode::InstructionId::INT_SUB, ircode::OperationId::INT_SUB },
    { pcode::InstructionId::INT_CARRY, ircode::OperationId::INT_CARRY },
    { pcode::InstructionId::INT_SCARRY, ircode::OperationId::INT_SCARRY },
    { pcode::InstructionId::INT_SBORROW, ircode::OperationId::INT_SBORROW },
    { pcode::InstructionId::INT_2COMP, ircode::OperationId::INT_2COMP },
    { pcode::InstructionId::INT_MULT, ircode::OperationId::INT_MULT },
    { pcode::InstructionId::INT_DIV, ircode::OperationId::INT_DIV },
    { pcode::InstructionId::INT_SDIV, ircode::OperationId::INT_SDIV },
    { pcode::InstructionId::INT_REM, ircode::OperationId::INT_REM },
    { pcode::InstructionId::INT_SREM, ircode::OperationId::INT_SREM },

    // Logical
    { pcode::InstructionId::INT_NEGATE, ircode::OperationId::INT_NEGATE },
    { pcode::InstructionId::INT_XOR, ircode::OperationId::INT_XOR },
    { pcode::InstructionId::INT_AND, ircode::OperationId::INT_AND },
    { pcode::InstructionId::INT_OR, ircode::OperationId::INT_OR },
    { pcode::InstructionId::INT_LEFT, ircode::OperationId::INT_LEFT },
    { pcode::InstructionId::INT_RIGHT, ircode::OperationId::INT_RIGHT },
    { pcode::InstructionId::INT_SRIGHT, ircode::OperationId::INT_SRIGHT },

    // Integer Comparison
    { pcode::InstructionId::INT_EQUAL, ircode::OperationId::INT_EQUAL },
    { pcode::InstructionId::INT_NOTEQUAL, ircode::OperationId::INT_NOTEQUAL },
    { pcode::InstructionId::INT_SLESS, ircode::OperationId::INT_SLESS },
    { pcode::InstructionId::INT_SLESSEQUAL, ircode::OperationId::INT_SLESSEQUAL },
    { pcode::InstructionId::INT_LESS, ircode::OperationId::INT_LESS },
    { pcode::InstructionId::INT_LESSEQUAL, ircode::OperationId::INT_LESSEQUAL },

    // Boolean
    { pcode::InstructionId::BOOL_NEGATE, ircode::OperationId::BOOL_NEGATE },
    { pcode::InstructionId::BOOL_XOR, ircode::OperationId::BOOL_XOR },
    { pcode::InstructionId::BOOL_AND, ircode::OperationId::BOOL_AND },
    { pcode::InstructionId::BOOL_OR, ircode::OperationId::BOOL_OR },

    // Floating point
    { pcode::InstructionId::FLOAT_ADD, ircode::OperationId::FLOAT_ADD },
    { pcode::InstructionId::FLOAT_SUB, ircode::OperationId::FLOAT_SUB },
    { pcode::InstructionId::FLOAT_MULT, ircode::OperationId::FLOAT_MULT },
    { pcode::InstructionId::FLOAT_DIV, ircode::OperationId::FLOAT_DIV },
    { pcode::InstructionId::FLOAT_NEG, ircode::OperationId::FLOAT_NEG },
    { pcode::InstructionId::FLOAT_ABS, ircode::OperationId::FLOAT_ABS },
    { pcode::InstructionId::FLOAT_SQRT, ircode::OperationId::FLOAT_SQRT },

    // Floating point compare
    { pcode::InstructionId::FLOAT_NAN, ircode::OperationId::FLOAT_NAN },
    { pcode::InstructionId::FLOAT_EQUAL, ircode::OperationId::FLOAT_EQUAL },
    { pcode::InstructionId::FLOAT_NOTEQUAL, ircode::OperationId::FLOAT_NOTEQUAL },
    { pcode::InstructionId::FLOAT_LESS, ircode::OperationId::FLOAT_LESS },
    { pcode::InstructionId::FLOAT_LESSEQUAL, ircode::OperationId::FLOAT_LESSEQUAL },
};

void IRcodeBlockGenerator::executePcode(pcode::Instruction* instr) {
    /* TODO:
        - сделать банальное преобразование PCode в IRcode;
        - вычислять хеши, учитывая комутативность операций (add, mul);
        - учитывать небольшую проблему испольования значений во время чтения памяти;
    */
    if (instr->getId() == pcode::InstructionId::NONE || instr->getId() == pcode::InstructionId::UNKNOWN)
        return;

    std::shared_ptr<ircode::Value> inputVal1;
    std::shared_ptr<ircode::Value> inputVal2;
    if (auto input0 = instr->getInput0()) {
        inputVal1 = genReadVarnode(input0.get());
    } else {
        assert(false && "Invalid instruction");
    }
    if (auto input1 = instr->getInput1()) {
        inputVal2 = genReadVarnode(input1.get());
    }

    ircode::MemoryAddress outputMemAddr;
    if (auto output = instr->getOutput()) {
        if (auto regVarnode = std::dynamic_pointer_cast<pcode::RegisterVarnode>(output)) {
            outputMemAddr.base = createRegister(regVarnode.get());
            outputMemAddr.offset = getRegisterMemoryInfo(regVarnode.get()).offset;
        } else {
            assert(false && "Output varnode is not a register");
        }
    }

    auto it = InstructionToOperation.find(instr->getId());
    if (it != InstructionToOperation.end()) {
        auto [instrId, operationId] = *it;

        // calculate hash
        ircode::Hash hash;
        if (instr->isComutative()) {
            if (instrId == pcode::InstructionId::INT_ADD) {
                hash = inputVal1->getHash() + inputVal2->getHash();
            } if (instrId == pcode::InstructionId::INT_MULT) {
                hash = inputVal1->getHash() * inputVal2->getHash();
            } else {
                boost::hash_combine(hash, operationId);
                boost::hash_combine(hash, inputVal1->getHash() + inputVal2->getHash());
            }
        } else {
            boost::hash_combine(hash, operationId);
            boost::hash_combine(hash, inputVal1->getHash());
            if (inputVal2) {
                boost::hash_combine(hash, inputVal2->getHash());
            }
        }
        auto outputMemSpace = m_totalMemSpace->getMemSpace(outputMemAddr.base->getHash());
        auto outputVar = createVariable(outputMemAddr, hash, outputMemAddr.base->getSize());
        genWriteMemory(outputMemSpace, outputVar);

        if (inputVal2) {
            genOperation(std::make_unique<ircode::BinaryOperation>(operationId, inputVal1, inputVal2, outputVar));
        } else {
            genOperation(std::make_unique<ircode::UnaryOperation>(operationId, inputVal1, outputVar));
        }

        // calculate address as linear expression
        const auto& linearExprInp1 = inputVal1->getLinearExpr();
        const auto& linearExprInp2 = inputVal2->getLinearExpr();
        if (instrId == pcode::InstructionId::INT_ADD) {
            outputVar->getLinearExpr() = linearExprInp1 + linearExprInp2;
        } else if (instrId == pcode::InstructionId::INT_MULT) {
            if (linearExprInp1.getTerms().empty() || linearExprInp2.getTerms().empty())
                outputVar->getLinearExpr() = linearExprInp1 * linearExprInp2;
        }
    } else {
        auto isCopyInstr = instr->getId() == pcode::InstructionId::COPY;
        auto isLoadInstr = instr->getId() == pcode::InstructionId::LOAD;
        if (isCopyInstr || isLoadInstr) {
            auto operationId = ircode::OperationId::COPY;
            auto inputVal = inputVal1;
            if (isLoadInstr || isCopyInstr && inputVal1->getType() == ircode::Value::Register) {
                auto baseAddrValue = inputVal1->getLinearExpr().getTerms().front().value;
                assert(baseAddrValue->getSize() == 8 && "Invalid address size");
                auto baseAddrHash = baseAddrValue->getHash();
                auto loadOffset = inputVal1->getLinearExpr().getConstTermValue();
                auto loadSize = outputMemAddr.base->getSize();
                const auto& [ baseAddrHash, readOffset ] = getRegisterMemoryInfo(regVarnode);

                // if it is not array, then use variable from memory
                if (inputVal1->getLinearExpr().getTerms().size() == 1) {
                    auto memSpace = m_totalMemSpace->getMemSpace(baseAddrHash);
                    auto readMask = BitMask(loadSize, 0);
                    auto varReadInfos = genReadMemory(memSpace, loadOffset, loadSize, readMask);
                }
            }

            ircode::Hash hash;
            boost::hash_combine(hash, operationId);
            boost::hash_combine(hash, inputVal->getHash());

            auto outputMemSpace = m_totalMemSpace->getMemSpace(outputMemAddr.base->getHash());
            auto outputVar = createVariable(outputMemAddr, hash, outputMemAddr.base->getSize());
            genWriteMemory(outputMemSpace, outputVar);

            genOperation(std::make_unique<ircode::UnaryOperation>(operationId, inputVal, outputVar));
        }
        else if (instr->getId() == pcode::InstructionId::STORE) {

        }
    }
}

void IRcodeBlockGenerator::genWriteMemory(MemorySpace* memSpace, std::shared_ptr<ircode::Variable> newVariable) {
    auto& variables = memSpace->variables;
    auto newVarOffset = newVariable->getMemAddress().offset;
    auto newVarSize = newVariable->getSize();

    // write rax -> remove eax/ax/ah/al
    auto it = variables.begin();
    while(it != variables.end()) {
        auto varOffset = (*it)->getMemAddress().offset;
        auto varSize = (*it)->getSize();
        // check full intersection
        if (varOffset >= newVarOffset && varOffset + varSize <= newVarOffset + newVarSize) {
            it = variables.erase(it);
        } else {
            ++it;
        }
    }

    memSpace->variables.push_front(newVariable);
}

std::list<IRcodeBlockGenerator::VariableReadInfo> IRcodeBlockGenerator::genReadMemory(MemorySpace* memSpace, size_t readOffset, size_t readSize, BitMask& readMask) {
    std::list<VariableReadInfo> varReadInfos;

    for (auto variable : memSpace->variables) {
        auto varOffset = variable->getMemAddress().offset;
        auto varSize = variable->getSize();
        // if not intersecting, skip
        if (varOffset + varSize <= readOffset || readOffset + readSize <= varOffset)
            continue;

        // for boundary intersection
        auto startDelta = (int64_t)varOffset - (int64_t)readOffset;
        auto endDelta = (int64_t)(varOffset + varSize) - (int64_t)(readOffset + readSize);
        
        BitMask varMask = 0;
        size_t extractOffset = 0;
        size_t extractSize = 0;

        // see different cases
        if (startDelta >= 0) {
            if (endDelta <= 0) {
                // ----|--====--|----
                varMask = BitMask(varSize, startDelta * 0x8);
            } else {
                // --==|==------|----
                extractOffset = 0;
                extractSize = varSize - endDelta;
                varMask = BitMask(extractSize, startDelta * 0x8);
            }
        } else {
            if (endDelta <= 0) {
                // ----|------==|==--
                extractOffset = -startDelta;
                extractSize = varSize + startDelta;
                varMask = BitMask(extractSize, 0);
            } else {
                // --==|========|==--
                extractOffset = -startDelta;
                extractSize = readSize;
                varMask = BitMask(extractSize, 0);
            }
        }

        // does the new mask intersect with the old one?
        auto newReadMask = readMask & ~varMask;
        if (newReadMask != readMask) {
            auto resultVariable = variable;
            if (extractSize != 0) {
                auto memAddress = variable->getMemAddress();
                memAddress.offset += extractOffset;

                ircode::Hash hash;
                boost::hash_combine(hash, ircode::OperationId::EXTRACT);
                boost::hash_combine(hash, variable->getHash());
                boost::hash_combine(hash, extractOffset);
                boost::hash_combine(hash, extractSize);

                resultVariable = createVariable(memAddress, hash, extractSize);
                genOperation(std::make_unique<ircode::ExtractOperation>(variable, extractOffset, resultVariable));
            }

            varReadInfos.push_front({ resultVariable, varMask.getOffset() / 0x8 });

            readMask = newReadMask;
            if (readMask == 0)
                break;
        }
    }

    return varReadInfos;
}

IRcodeBlockGenerator::MemoryInfo IRcodeBlockGenerator::getRegisterMemoryInfo(const pcode::RegisterVarnode* regVarnode) const {
    auto baseAddrHash = std::hash<size_t>()(regVarnode->getRegId());
    size_t offset = 0;
    if (regVarnode->getRegType() == pcode::RegisterVarnode::Virtual) {
        boost::hash_combine(baseAddrHash, regVarnode->getRegIndex());
    } else {
        offset = regVarnode->getRegIndex() * 0x8 + regVarnode->getMask().getOffset() / 0x8;
    }
    return { baseAddrHash, offset };
}

std::list<IRcodeBlockGenerator::VariableReadInfo> IRcodeBlockGenerator::genReadRegisterVarnode(const pcode::RegisterVarnode* regVarnode) {
    const auto& [ baseAddrHash, readOffset ] = getRegisterMemoryInfo(regVarnode);
    auto memSpace = m_totalMemSpace->getMemSpace(baseAddrHash);
    auto readSize = regVarnode->getSize();
    auto readMask = BitMask(readSize, 0);

    auto varReadInfos = genReadMemory(memSpace, readOffset, readSize, readMask);

    if (readMask != 0) { // todo: readMask can be 0xFF00FF00, which is not a valid mask
        // todo: see case when request eax, then rax
        auto regValue = createRegister(regVarnode);

        ircode::Hash hash;
        boost::hash_combine(hash, ircode::OperationId::LOAD);
        boost::hash_combine(hash, regValue->getHash());

        auto regVariable = createVariable(ircode::MemoryAddress { regValue, readOffset }, hash, readSize);
        genOperation(std::make_unique<ircode::UnaryOperation>(ircode::OperationId::LOAD, regValue, regVariable));
        memSpace->variables.push_back(regVariable);

        varReadInfos.push_front({ regVariable, 0 });
    }

    return varReadInfos;
}

std::shared_ptr<ircode::Value> IRcodeBlockGenerator::genReadVarnode(const pcode::Varnode* varnode) {
    if (auto regVarnode = dynamic_cast<const pcode::RegisterVarnode*>(varnode)) {
        auto varReadInfos = genReadRegisterVarnode(regVarnode);

        auto concatVariable = varReadInfos.front().variable;
        varReadInfos.pop_front();
        for (const auto& varReadInfo : varReadInfos) {
            ircode::Hash hash;
            boost::hash_combine(hash, ircode::OperationId::CONCAT);
            boost::hash_combine(hash, concatVariable->getHash());
            boost::hash_combine(hash, varReadInfo.variable->getHash());
            boost::hash_combine(hash, varReadInfo.offset);
            boost::hash_combine(hash, regVarnode->getSize());
            
            auto newConcatVariable = createVariable(ircode::MemoryAddress(), hash, regVarnode->getSize());
            genOperation(std::make_unique<ircode::ConcatOperation>(
                concatVariable,
                varReadInfo.variable,
                varReadInfo.offset,
                newConcatVariable));
            concatVariable = newConcatVariable;
        }

        return concatVariable;
    }
    else if (auto constVarnode = dynamic_cast<const pcode::ConstantVarnode*>(varnode)) {
        return createConstant(constVarnode);
    }
    
    assert(false && "unsupported varnode type");
}

void IRcodeBlockGenerator::genOperation(std::unique_ptr<ircode::Operation> operation) {
    m_block->getOperations().push_back(std::move(operation));
}

std::shared_ptr<ircode::Constant> IRcodeBlockGenerator::createConstant(const pcode::ConstantVarnode* constVarnode) const {
    auto hash = std::hash<size_t>()(constVarnode->getValue());
    boost::hash_combine(hash, ircode::Value::Constant);
    auto value = std::make_shared<ircode::Constant>(constVarnode, hash);
    value->getLinearExpr() = ircode::LinearExpression(value);
    return value;
}

std::shared_ptr<ircode::Register> IRcodeBlockGenerator::createRegister(const pcode::RegisterVarnode* regVarnode) const {
    auto [ hash, readOffset ] = getRegisterMemoryInfo(regVarnode);
    boost::hash_combine(hash, readOffset);
    boost::hash_combine(hash, regVarnode->getSize());
    boost::hash_combine(hash, ircode::Value::Register);
    auto value = std::make_shared<ircode::Register>(regVarnode, hash);
    value->getLinearExpr() = ircode::LinearExpression(value);
    return value;
}

std::shared_ptr<ircode::Variable> IRcodeBlockGenerator::createVariable(const ircode::MemoryAddress& memAddress, ircode::Hash hash, size_t size) const {
    boost::hash_combine(hash, ircode::Value::Variable);
    auto value = std::make_shared<ircode::Variable>(memAddress, hash, size);
    value->getLinearExpr() = ircode::LinearExpression(value);
    return value;
}