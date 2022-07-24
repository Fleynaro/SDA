#include "Decompiler/IRcodeGenerator/IRcodeBlockGenerator.h"
#include <boost/functional/hash.hpp>

using namespace sda;
using namespace sda::decompiler;

MemorySpace* MemorySpacePool::getMemSpace(ircode::Hash baseAddrHash) {
    auto it = m_memorySpaces.find(baseAddrHash);
    if (it == m_memorySpaces.end()) {
        m_memorySpaces[baseAddrHash] = MemorySpace();
        return &m_memorySpaces[baseAddrHash];
    }
    return &it->second;
}

IRcodeBlockGenerator::IRcodeBlockGenerator(ircode::Block* block, MemorySpacePool* memSpacePool)
    : m_block(block), m_memSpacePool(memSpacePool)
{}

void IRcodeBlockGenerator::executePcode(pcode::Instruction* instr) {

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
        } else  {
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

                resultVariable = std::make_shared<ircode::Variable>(memAddress, hash, extractSize);
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

std::list<IRcodeBlockGenerator::VariableReadInfo> IRcodeBlockGenerator::genReadRegisterVarnode(const pcode::RegisterVarnode* regVarnode) {
    auto baseAddrHash = std::hash<size_t>()(regVarnode->getRegId());
    if (regVarnode->getRegType() == pcode::RegisterVarnode::Virtual) {
        boost::hash_combine(baseAddrHash, (std::uintptr_t)regVarnode);
    }
    auto memSpace = m_memSpacePool->getMemSpace(baseAddrHash);
    auto readOffset = regVarnode->getRegIndex() * 0x8 + regVarnode->getMask().getOffset() / 0x8;
    auto readSize = regVarnode->getSize();
    auto readMask = BitMask(readSize, 0);

    auto varReadInfos = genReadMemory(memSpace, readOffset, readSize, readMask);

    if (readMask != 0) { // todo: readMask can be 0xFF00FF00, which is not a valid mask
        // todo: request eax, then rax
        ircode::MemoryAddress memAddress;
        memAddress.base = std::make_shared<ircode::Register>(regVarnode, baseAddrHash);
        memAddress.offset = readOffset;

        ircode::Hash hash;
        boost::hash_combine(hash, ircode::OperationId::LOAD);
        boost::hash_combine(hash, baseAddrHash);
        boost::hash_combine(hash, readSize);

        auto regVariable = std::make_shared<ircode::Variable>(memAddress, hash, readSize);
        genOperation(std::make_unique<ircode::UnaryOperation>(ircode::OperationId::LOAD, memAddress.base, regVariable));
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
            
            auto newConcatVariable = std::make_shared<ircode::Variable>(ircode::MemoryAddress(), hash, regVarnode->getSize());
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
        auto hash = std::hash<size_t>()(constVarnode->getValue());
        return std::make_shared<ircode::Constant>(constVarnode, hash);
    }
    
    throw std::runtime_error("unsupported varnode type");
}

void IRcodeBlockGenerator::genOperation(std::unique_ptr<ircode::Operation> operation) {
    m_block->getOperations().push_back(std::move(operation));
}