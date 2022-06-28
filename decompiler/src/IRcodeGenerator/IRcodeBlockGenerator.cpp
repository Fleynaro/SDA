#include "Decompiler/IRcodeGenerator/IRcodeBlockGenerator.h"
#include <boost/functional/hash.hpp>

using namespace sda;
using namespace sda::decompiler;

IRcodeBlockGenerator::IRcodeBlockGenerator(ircode::Block* block)
    : m_block(block)
{}

void IRcodeBlockGenerator::executePcode(pcode::Instruction* instr) {

}

std::list<IRcodeBlockGenerator::VariableReadInfo> IRcodeBlockGenerator::readMemory(MemorySpace* memSpace, size_t readOffset, size_t readSize, BitMask& readMask) {
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
            if (extractSize != 0) {
                auto memAddress = variable->getMemAddress();
                memAddress.offset += extractOffset;

                ircode::Hash hash;
                boost::hash_combine(hash, ircode::OperationId::EXTRACT);
                boost::hash_combine(hash, variable->getHash());
                boost::hash_combine(hash, extractOffset);
                boost::hash_combine(hash, extractSize);

                auto newVariable = std::make_shared<ircode::Variable>(memAddress, hash, extractSize);
                addOperation(std::make_unique<ircode::ExtractOperation>(variable, extractOffset, extractSize, newVariable));
                varReadInfos.push_back({ newVariable, varMask });
            } else {
                varReadInfos.push_back({ variable, varMask });
            }

            readMask = newReadMask;
            if (readMask == 0)
                break;
        }
    }

    return varReadInfos;
}

std::list<IRcodeBlockGenerator::VariableReadInfo> IRcodeBlockGenerator::readRegisterVarnode(const pcode::RegisterVarnode* regVarnode) {
    auto baseAddrHash = std::hash<size_t>()(regVarnode->getRegId()); // todo: virtual reg
    auto memSpace = getMemSpace(baseAddrHash);
    auto readOffset = regVarnode->getRegIndex() * 0x8 + regVarnode->getMask().getOffset() / 0x8;
    auto readSize = regVarnode->getSize();
    auto readMask = BitMask(readSize, 0);
    auto varReadInfos = readMemory(memSpace, readOffset, readSize, readMask); // todo: readMask can be 0xFF00FF00

    return varReadInfos;
}

std::shared_ptr<ircode::Value> IRcodeBlockGenerator::readVarnode(const pcode::Varnode* varnode) {
    if (auto regVarnode = dynamic_cast<const pcode::RegisterVarnode*>(varnode)) {
        auto varReadInfos = readRegisterVarnode(regVarnode);

    }
    else if (auto constVarnode = dynamic_cast<const pcode::ConstantVarnode*>(varnode)) {
        auto hash = std::hash<size_t>()(constVarnode->getValue());
        return std::make_shared<ircode::Constant>(constVarnode, hash);
    }
}

IRcodeBlockGenerator::MemorySpace* IRcodeBlockGenerator::getMemSpace(ircode::Hash baseAddrHash) {
    auto it = m_memorySpaces.find(baseAddrHash);
    if (it == m_memorySpaces.end()) {
        m_memorySpaces[baseAddrHash] = MemorySpace();
        return &m_memorySpaces[baseAddrHash];
    }
    return &it->second;
}

void IRcodeBlockGenerator::addOperation(std::unique_ptr<ircode::Operation> operation) {
    m_block->getOperations().push_back(std::move(operation));
}