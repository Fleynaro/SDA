#include "Decompiler/IRcode/Generator/IRcodeBlockGenerator.h"
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

IRcodeBlockGenerator::IRcodeBlockGenerator(
    ircode::Block* block,
    TotalMemorySpace* totalMemSpace,
    IRcodeDataTypePropagator* dataTypePropagator,
    size_t nextVarId)
    : m_block(block),
    m_totalMemSpace(totalMemSpace),
    m_dataTypePropagator(dataTypePropagator),
    m_nextVarId(nextVarId)
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

void IRcodeBlockGenerator::executePcode(const pcode::Instruction* instr) {
    m_curInstr = instr;
    if (instr->getId() == pcode::InstructionId::NONE || instr->getId() == pcode::InstructionId::UNKNOWN)
        return;

    // get output address (this is always a register)
    ircode::MemoryAddress outputMemAddr;
    if (auto output = instr->getOutput()) {
        if (auto regVarnode = std::dynamic_pointer_cast<pcode::RegisterVarnode>(output)) {
            outputMemAddr = getRegisterMemoryAddress(regVarnode->getRegister());
            outputMemAddr.value = createRegister(regVarnode.get(), outputMemAddr);
        } else {
            assert(false && "Output varnode is not a register");
        }
    }

    auto it = InstructionToOperation.find(instr->getId());
    if (it != InstructionToOperation.end()) {
        genGenericOperation(instr, it->second, outputMemAddr);
    } else {
        auto isCopyInstr = instr->getId() == pcode::InstructionId::COPY;
        auto isLoadInstr = instr->getId() == pcode::InstructionId::LOAD;
        auto isStoreInstr = instr->getId() == pcode::InstructionId::STORE;

        if (isCopyInstr && !instr->getInput0()->isRegister()) {
            // constant value copying (RAX:4 = COPY 100:4)
            genGenericOperation(instr, ircode::OperationId::COPY, outputMemAddr);
        }
        else if (isStoreInstr && !instr->getInput1()->isRegister()) {
            // constant value storing (STORE [RAX:8], 100:4)
            genGenericOperation(instr, ircode::OperationId::COPY, outputMemAddr);
        }
        else if (isCopyInstr || isLoadInstr || isStoreInstr) {
            std::list<IRcodeBlockGenerator::VariableReadInfo> varReadInfos;

            // 1) just get values from memory or generate load operations otherwise
            if (isCopyInstr) {
                // register value copying (RAX:4 = COPY RCX:4)
                if (auto regVarnode = std::dynamic_pointer_cast<pcode::RegisterVarnode>(instr->getInput0())) {
                    varReadInfos = genReadRegisterVarnode(regVarnode.get());
                }
            }
            else if (isStoreInstr) {
                // register value storing (STORE [RAX:8], RCX:4)
                if (auto regVarnode = std::dynamic_pointer_cast<pcode::RegisterVarnode>(instr->getInput1())) {
                    varReadInfos = genReadRegisterVarnode(regVarnode.get());
                }

                // get destination address value ([RCX:8 + 0x10:8])
                std::shared_ptr<ircode::Value> addrValue;
                if (auto input0 = instr->getInput0()) {
                    addrValue = genReadVarnode(input0.get());
                }

                // parse destination address value into base and offset (RCX:8 and 0x10:8)
                outputMemAddr = getMemoryAddress(addrValue);
            } 
            else if (isLoadInstr) {
                // memory value copying (RAX:4 = LOAD [RCX:8 + 0x10:8])

                // get source address value ([RCX:8 + 0x10:8])
                std::shared_ptr<ircode::Value> addrValue;
                if (auto input0 = instr->getInput0()) {
                    addrValue = genReadVarnode(input0.get());
                }

                // parse source address value into base and offset (RCX:8 and 0x10:8)
                auto memAddr = getMemoryAddress(addrValue);

                // get memory space and load size
                auto loadSize = instr->getOutput()->getSize();
                auto memSpace = m_totalMemSpace->getMemSpace(memAddr.baseAddrHash);

                // check if it is an array
                if (!memAddr.isDynamic()) {
                    // see non-array case
                    auto readMask = BitMask(loadSize, 0);
                    varReadInfos = genReadMemory(memSpace, memAddr.offset, loadSize, readMask);
                    if (readMask != 0) {
                        // this logic is similar to genReadRegisterVarnode()
                        auto outVariable = genLoadOperation(memAddr, loadSize);
                        memSpace->variables.push_back(outVariable);
                        varReadInfos.push_front({ outVariable, 0 });
                    }
                } else {
                    // see array case (e.g. RAX:4 = LOAD [RCX:8 + RDX:8 * 0x4:8 + 0x10:8])
                    auto outVariable = genLoadOperation(memAddr, loadSize);
                    varReadInfos.push_front({ outVariable, 0 });
                }
            }

            // 2) generate copy operations
            for (const auto& varReadInfo : varReadInfos) {
                auto srcVar = varReadInfo.variable;

                auto dstMemAddr = outputMemAddr;
                dstMemAddr.offset = varReadInfo.offset;
                auto dstMemSpace = m_totalMemSpace->getMemSpace(dstMemAddr.baseAddrHash);
                auto dstVar = createVariable(dstMemAddr, srcVar->getHash(), dstMemAddr.value->getSize());
                genWriteMemory(dstMemSpace, dstVar);
                genOperation(std::make_unique<ircode::UnaryOperation>(ircode::OperationId::COPY, srcVar, dstVar));
            }
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

        // check intersection (full or partial)
        if (varOffset < newVarOffset + newVarSize && varOffset + varSize > newVarOffset) {
            m_overwrittenVariables.insert(*it);
        }

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

ircode::MemoryAddress IRcodeBlockGenerator::getRegisterMemoryAddress(const pcode::Register& reg) const {
    auto baseAddrHash = std::hash<size_t>()(reg.getRegId());
    size_t offset = 0;
    if (reg.getRegType() == pcode::Register::Virtual) {
        boost::hash_combine(baseAddrHash, reg.getRegIndex());
    } else {
        offset = reg.getRegIndex() * 0x8 + reg.getMask().getOffset() / 0x8;
    }
    return { nullptr, baseAddrHash, offset };
}

ircode::MemoryAddress IRcodeBlockGenerator::getMemoryAddress(std::shared_ptr<ircode::Value> addrValue) const {
    const auto& addrExpr = addrValue->getLinearExpr();
    auto baseAddrValue = addrExpr.getBaseValue();
    assert(baseAddrValue->getSize() == 8 && "Invalid address size");
    return {
        addrValue,
        baseAddrValue->getHash(),
        addrExpr.getConstTermValue()
    };
}

std::list<IRcodeBlockGenerator::VariableReadInfo> IRcodeBlockGenerator::genReadRegisterVarnode(const pcode::RegisterVarnode* regVarnode) {
    auto memAddr = getRegisterMemoryAddress(regVarnode->getRegister());
    auto memSpace = m_totalMemSpace->getMemSpace(memAddr.baseAddrHash);
    auto readSize = regVarnode->getSize();
    auto readMask = BitMask(readSize, 0);

    auto varReadInfos = genReadMemory(memSpace, memAddr.offset, readSize, readMask);

    if (readMask != 0) { // todo: readMask can be 0xFF00FF00, which is not a valid mask
        // todo: see case when request eax, then rax
        memAddr.value = createRegister(regVarnode, memAddr);
        auto outVariable = genLoadOperation(memAddr, readSize);
        memSpace->variables.push_back(outVariable);
        varReadInfos.push_front({ outVariable, 0 });
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
    
    throw std::runtime_error("Invalid varnode type");
}

void IRcodeBlockGenerator::genOperation(std::unique_ptr<ircode::Operation> operation) {
    m_dataTypePropagator->propagate(operation.get());
    operation->getPcodeInstructions().insert(m_curInstr);
    operation->getOverwrittenVariables() = m_overwrittenVariables;
    m_overwrittenVariables.clear();
    m_block->getOperations().push_back(std::move(operation));
}

void IRcodeBlockGenerator::genGenericOperation(const pcode::Instruction* instr, ircode::OperationId operationId, ircode::MemoryAddress& outputMemAddr) {
    // get input values
    std::shared_ptr<ircode::Value> inputVal1;
    std::shared_ptr<ircode::Value> inputVal2;
    if (auto input0 = instr->getInput0())
        inputVal1 = genReadVarnode(input0.get());
    if (auto input1 = instr->getInput1())
        inputVal2 = genReadVarnode(input1.get());
    if (!inputVal1)
        throw std::runtime_error("Invalid instruction");

    // exception for STORE instruction
    if (instr->getId() == pcode::InstructionId::STORE) {
        outputMemAddr = getMemoryAddress(inputVal1);
        inputVal1 = inputVal2;
        inputVal2 = nullptr;
    }
    
    // calculate hash
    ircode::Hash hash;
    if (instr->isComutative()) {
        // INT_ADD, INT_MULT used for address calculation ([x+4y]*2 == 2x+8y == 8y+2x)
        if (instr->getId() == pcode::InstructionId::INT_ADD) {
            hash = inputVal1->getHash() + inputVal2->getHash();
        } else if (instr->getId() == pcode::InstructionId::INT_MULT) {
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
    // create variable and save it to the memory space
    auto outputMemSpace = m_totalMemSpace->getMemSpace(outputMemAddr.baseAddrHash);
    auto outputVar = createVariable(outputMemAddr, hash, outputMemAddr.value->getSize());
    genWriteMemory(outputMemSpace, outputVar);

    // generate operation
    if (inputVal2) {
        genOperation(std::make_unique<ircode::BinaryOperation>(operationId, inputVal1, inputVal2, outputVar));

        // calculate address as linear expression
        const auto& linearExprInp1 = inputVal1->getLinearExpr();
        const auto& linearExprInp2 = inputVal2->getLinearExpr();
        if (instr->getId() == pcode::InstructionId::INT_ADD) {
            outputVar->getLinearExpr() = linearExprInp1 + linearExprInp2;
        } else if (instr->getId() == pcode::InstructionId::INT_MULT) {
            if (linearExprInp1.getTerms().empty() || linearExprInp2.getTerms().empty())
                outputVar->getLinearExpr() = linearExprInp1 * linearExprInp2;
        }
    } else {
        genOperation(std::make_unique<ircode::UnaryOperation>(operationId, inputVal1, outputVar));
    }
}

std::shared_ptr<ircode::Variable> IRcodeBlockGenerator::genLoadOperation(const ircode::MemoryAddress& memAddr, size_t loadSize) {
    ircode::Hash hash;
    boost::hash_combine(hash, ircode::OperationId::LOAD);
    boost::hash_combine(hash, memAddr.value->getHash());

    auto regVariable = createVariable(memAddr, hash, loadSize);
    genOperation(std::make_unique<ircode::UnaryOperation>(ircode::OperationId::LOAD, memAddr.value, regVariable));
    return regVariable;
}

std::shared_ptr<ircode::Constant> IRcodeBlockGenerator::createConstant(const pcode::ConstantVarnode* constVarnode) const {
    auto hash = std::hash<size_t>()(constVarnode->getValue());
    boost::hash_combine(hash, ircode::Value::Constant);
    auto value = std::make_shared<ircode::Constant>(constVarnode, hash);
    value->getLinearExpr() = ircode::LinearExpression(value->getConstVarnode()->getValue());
    return value;
}

std::shared_ptr<ircode::Register> IRcodeBlockGenerator::createRegister(const pcode::RegisterVarnode* regVarnode, const ircode::MemoryAddress& memAddr) const {
    auto hash = memAddr.baseAddrHash;
    boost::hash_combine(hash, memAddr.offset);
    boost::hash_combine(hash, regVarnode->getSize());
    boost::hash_combine(hash, ircode::Value::Register);
    auto value = std::make_shared<ircode::Register>(regVarnode, hash);
    value->getLinearExpr() = ircode::LinearExpression(value);
    return value;
}

std::shared_ptr<ircode::Variable> IRcodeBlockGenerator::createVariable(const ircode::MemoryAddress& memAddress, ircode::Hash hash, size_t size) {
    boost::hash_combine(hash, size);
    boost::hash_combine(hash, ircode::Value::Variable);
    auto value = std::make_shared<ircode::Variable>(m_nextVarId++, memAddress, hash, size);
    value->getLinearExpr() = ircode::LinearExpression(value);
    return value;
}