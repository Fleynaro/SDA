#include "SDA/Core/IRcode/IRcodeGenerator.h"
#include "SDA/Core/IRcode/IRcodeFunction.h"
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"
#include "SDA/Core/Platform/RegisterRepository.h"

using namespace sda;
using namespace sda::ircode;

IRcodeGenerator::IRcodeGenerator(
    Block* block,
    ircode::DataTypeProvider* dataTypeProvider,
    std::function<size_t()> nextVarIdProvider)
    : m_block(block),
    m_dataTypeProvider(dataTypeProvider),
    m_nextVarIdProvider(nextVarIdProvider)
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

void IRcodeGenerator::ingestPcode(const pcode::Instruction* instr) {
    m_curInstr = instr;
    m_genOperations.clear();
    if (instr->getId() == pcode::InstructionId::NONE || instr->getId() == pcode::InstructionId::UNKNOWN)
        return;

    // get output address (this is always a register)
    ircode::MemoryAddress outputMemAddr;
    if (auto output = instr->getOutput()) {
        if (auto regVarnode = std::dynamic_pointer_cast<pcode::RegisterVarnode>(output)) {
            outputMemAddr = getRegisterMemoryAddress(regVarnode);
        } else {
            assert(false && "Output varnode is not a register");
        }
    }

    auto it = InstructionToOperation.find(instr->getId());
    if (it != InstructionToOperation.end()) {
        genGenericOperation(instr, it->second, outputMemAddr);
    } else if (instr->getId() == pcode::InstructionId::CALL) {
        genCallOperation(instr);
    } else if (instr->getId() == pcode::InstructionId::CBRANCH) {
        handleConditionJumpOperation(instr);
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
            std::list<IRcodeGenerator::VariableReadInfo> varReadInfos;

            // 1) just get values from memory or generate load operations otherwise
            if (isCopyInstr) {
                // register value copying (RAX:4 = COPY RCX:4)
                if (auto regVarnode = std::dynamic_pointer_cast<pcode::RegisterVarnode>(instr->getInput0())) {
                    varReadInfos = genReadRegisterVarnode(regVarnode);
                }
            }
            else if (isStoreInstr) {
                // register value storing (STORE [RAX:8], RCX:4)
                if (auto regVarnode = std::dynamic_pointer_cast<pcode::RegisterVarnode>(instr->getInput1())) {
                    varReadInfos = genReadRegisterVarnode(regVarnode);
                }

                // get destination address value ([RCX:8 + 0x10:8])
                std::shared_ptr<ircode::Value> addrValue;
                if (auto input0 = instr->getInput0()) {
                    addrValue = genReadVarnode(input0);
                }

                // parse destination address value into base and offset (RCX:8 and 0x10:8)
                outputMemAddr = getMemoryAddress(addrValue);
            } 
            else if (isLoadInstr) {
                // memory value copying (RAX:4 = LOAD [RCX:8 + 0x10:8])

                // get source address value ([RCX:8 + 0x10:8])
                m_generationEnabled = false;
                auto addrValue = genReadVarnode(instr->getInput0());
                m_generationEnabled = true;

                // parse source address value into base and offset (RCX:8 and 0x10:8)
                auto memAddr = getMemoryAddress(addrValue);

                // get memory space and load size
                auto loadSize = instr->getOutput()->getSize();
                auto memSpace = getCurMemSpace()->getSubspace(memAddr.baseAddrHash);

                // check if it is an array
                if (!memAddr.value->getLinearExpr().isArrayType()) {
                    // see non-array case
                    auto readMask = utils::BitMask(loadSize, 0);
                    varReadInfos = genReadMemory(memAddr, loadSize, readMask);
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
                auto dstMemSpace = getCurMemSpace()->getSubspace(dstMemAddr.baseAddrHash);
                auto dstVar = createVariable(dstMemAddr, srcVar->getHash(), srcVar->getSize());
                dstVar->setLinearExpr(srcVar->getLinearExpr());
                genWriteMemory(dstMemSpace, dstVar);
                genOperation(std::make_unique<ircode::UnaryOperation>(ircode::OperationId::COPY, srcVar, dstVar));
            }
        }
    }
}

const std::list<ircode::Operation*>& IRcodeGenerator::getGeneratedOperations() const {
    return m_genOperations;
}

void IRcodeGenerator::genWriteMemory(MemorySubspace* memSpace, std::shared_ptr<ircode::Variable> newVariable, bool blockScope) {
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
            memSpace->blockScopedVars.erase(*it);
            it = variables.erase(it);
        } else {
            ++it;
        }
    }

    memSpace->variables.push_front(newVariable);
    if (blockScope) {
        memSpace->blockScopedVars.insert(newVariable);
    }
}

std::list<IRcodeGenerator::VariableReadInfo> IRcodeGenerator::genReadMemory(
    Block* block,
    const MemoryAddress& readMemAddr,
    size_t readSize,
    utils::BitMask& readMask
) {
    std::list<VariableReadInfo> varReadInfos;
    auto isCurrentBlock = block == m_block;
    auto baseAddrHash = readMemAddr.baseAddrHash;
    auto readOffset = readMemAddr.offset;
    auto memSpace = block->getMemorySpace()->getSubspace(baseAddrHash);

    for (auto variable : memSpace->variables) {
        //  if variable is block scoped and we are not in the same block, skip
        if (!isCurrentBlock && memSpace->blockScopedVars.find(variable) != memSpace->blockScopedVars.end())
            continue;
        auto varOffset = variable->getMemAddress().offset;
        auto varSize = variable->getSize();
        // if not intersecting, skip
        if (varOffset + varSize <= readOffset || readOffset + readSize <= varOffset)
            continue;

        // for boundary intersection
        auto startDelta = (int64_t)varOffset - (int64_t)readOffset;
        auto endDelta = (int64_t)(varOffset + varSize) - (int64_t)(readOffset + readSize);
        
        utils::BitMask varMask = 0;
        Offset extractOffset = 0;
        size_t extractSize = 0;

        // see different cases
        if (startDelta >= 0) {
            if (endDelta <= 0) {
                // ----|--====--|----
                varMask = utils::BitMask(varSize, startDelta);
            } else {
                // --==|==------|----
                extractOffset = 0;
                extractSize = varSize - endDelta;
                varMask = utils::BitMask(extractSize, startDelta);
            }
        } else {
            if (endDelta <= 0) {
                // ----|------==|==--
                extractOffset = -startDelta;
                extractSize = varSize + startDelta;
                varMask = utils::BitMask(extractSize, 0);
            } else {
                // --==|========|==--
                extractOffset = -startDelta;
                extractSize = readSize;
                varMask = utils::BitMask(extractSize, 0);
            }
        }

        // does the new mask intersect with the old one?
        auto newReadMask = readMask & ~varMask;
        if (newReadMask != readMask) {
            std::shared_ptr<Variable> resultVariable = variable;
            if (!isCurrentBlock) {
                // if the variable is from another block, we need to create a reference operation
                RefOperation::Reference ref = {
                    block,
                    baseAddrHash,
                    varOffset,
                    varSize
                };
                auto refVar = createVariable(ircode::MemoryAddress(), ref.getHash(), resultVariable->getSize());
                auto curMemSpace = getCurMemSpace()->getSubspace(baseAddrHash);
                //genWriteMemory(curMemSpace, resultVariable, true);
                genOperation(std::make_unique<ircode::RefOperation>(ref, resultVariable, refVar));
                resultVariable = refVar;
            }
            if (extractSize != 0) {
                // if we need to extract a part of the variable, we need to generate an extract operation
                auto inputVar = resultVariable;
                auto memAddress = ircode::MemoryAddress();
                memAddress.offset += extractOffset;

                ircode::Hash hash = 0;
                boost::hash_combine(hash, ircode::OperationId::EXTRACT);
                boost::hash_combine(hash, inputVar->getHash());
                boost::hash_combine(hash, extractOffset);

                resultVariable = createVariable(memAddress, hash, extractSize);
                genOperation(std::make_unique<ircode::ExtractOperation>(inputVar, extractOffset, resultVariable));
            }

            varReadInfos.push_front({ resultVariable, varMask.getOffset() / 0x8 });

            readMask = newReadMask;
            if (readMask == 0)
                break;
        }
    }

    return varReadInfos;
}

std::list<IRcodeGenerator::VariableReadInfo> IRcodeGenerator::genReadMemory(Block* block, utils::BitMask& readMask, BlockReadContext& ctx) {
    Hash cacheHash;
    boost::hash_combine(cacheHash, block);
    boost::hash_combine(cacheHash, (size_t)readMask);
    if (auto it = ctx.cache.find(cacheHash); it != ctx.cache.end()) {
        auto readInfo = it->second;
        readMask = readMask & ~readInfo.readMask;
        return readInfo.varReadInfos;
    }
    auto prevReadMask = readMask;
    auto varReadInfos = genReadMemory(
        block,
        ctx.memAddr,
        ctx.readSize,
        readMask);
    for (auto varReadInfo : varReadInfos) {
        varReadInfo.srcBlock = block;
    }
    if (readMask != 0) {
        std::list<VariableReadInfo> remainVarReadInfos;
        utils::BitMask remainReadMask(0);
        for (auto refBlock : block->getReferencedBlocks()) {
            if (ctx.visitedBlocks.get(refBlock->getIndex()))
                continue;
            auto refReadMask = readMask;
            ctx.visitedBlocks.set(refBlock->getIndex(), true);
            auto refVarReadInfos = genReadMemory(refBlock, refReadMask, ctx);
            ctx.visitedBlocks.set(refBlock->getIndex(), false);
            remainReadMask = remainReadMask | refReadMask;
            if (remainVarReadInfos.empty()) {
                remainVarReadInfos = refVarReadInfos;
            } else {
                if (refVarReadInfos != remainVarReadInfos) {
                    if (remainReadMask != 0) {
                        auto var = genLoadBackgroundValue(ctx);
                        remainVarReadInfos.push_front({ var, 0 });
                    }
                    if (refReadMask != 0) {
                        auto var = genLoadBackgroundValue(ctx);
                        refVarReadInfos.push_front({ var, 0 });
                    }
                    auto var1 = joinVariables(remainVarReadInfos, ctx.readSize);
                    auto var2 = joinVariables(refVarReadInfos, ctx.readSize);

                    ircode::Hash hash = 0;
                    boost::hash_combine(hash, ircode::OperationId::PHI);
                    boost::hash_combine(hash, var1->getHash());
                    boost::hash_combine(hash, var2->getHash());
                    auto phiVariable = createVariable(ircode::MemoryAddress(), hash, ctx.readSize);
                    genOperation(std::make_unique<ircode::PhiOperation>(
                        var1,
                        var2,
                        phiVariable));
                    remainVarReadInfos = { { phiVariable, 0, m_block } };
                    remainReadMask = 0;
                    // TODO: write in memory?
                }
            }
        }
        if (!remainVarReadInfos.empty()) {
            readMask = readMask & remainReadMask;
            varReadInfos.splice(varReadInfos.begin(), remainVarReadInfos);
        }
    }
    ctx.cache[cacheHash] = { varReadInfos, prevReadMask ^ readMask };
    return varReadInfos;
}

std::shared_ptr<ircode::Variable> IRcodeGenerator::genLoadBackgroundValue(BlockReadContext& ctx) {
    if (ctx.backgroundValue) {
        return ctx.backgroundValue;
    }
    auto var = genLoadOperation(ctx.memAddr, ctx.readSize);
    auto memSpace = getCurMemSpace()->getSubspace(ctx.memAddr.baseAddrHash);
    memSpace->variables.push_back(var);
    ctx.backgroundValue = var;
    return var;
}

std::list<IRcodeGenerator::VariableReadInfo> IRcodeGenerator::genReadMemory(
    const MemoryAddress& memAddr,
    size_t readSize,
    utils::BitMask& readMask
) {
    BlockReadContext ctx = {
        memAddr,
        readSize,
    };
    auto varReadInfos = genReadMemory(m_block, readMask, ctx);
    // TODO: see case when request eax, then rax
    // TODO: readMask can be 0xFF00FF00, which is not a valid mask
    if (readMask != 0) {
        auto var = genLoadBackgroundValue(ctx);
        varReadInfos.push_front({ var, 0 });
    }
    return varReadInfos;
}

std::shared_ptr<ircode::Variable> IRcodeGenerator::joinVariables(std::list<VariableReadInfo> varReadInfos, size_t size) {
    auto concatVariable = varReadInfos.front().variable;
    varReadInfos.pop_front();
    for (const auto& varReadInfo : varReadInfos) {
        ircode::Hash hash = 0;
        boost::hash_combine(hash, ircode::OperationId::CONCAT);
        boost::hash_combine(hash, concatVariable->getHash());
        boost::hash_combine(hash, varReadInfo.variable->getHash());
        boost::hash_combine(hash, varReadInfo.offset);
        
        auto newConcatVariable = createVariable(ircode::MemoryAddress(), hash, size);
        genOperation(std::make_unique<ircode::ConcatOperation>(
            concatVariable,
            varReadInfo.variable,
            varReadInfo.offset,
            newConcatVariable));
        concatVariable = newConcatVariable;
    }
    return concatVariable;
}

ircode::MemoryAddress IRcodeGenerator::getRegisterMemoryAddress(std::shared_ptr<pcode::RegisterVarnode> regVarnode) const {
    auto reg = regVarnode->getRegister();
    auto baseAddrHash = std::hash<size_t>()(reg.getRegId());
    Offset offset = 0;
    if (reg.getRegType() == sda::Register::Virtual) {
        boost::hash_combine(baseAddrHash, reg.getRegIndex());
    } else {
        offset = reg.getBitOffset() / utils::BitsInBytes;
    }
    MemoryAddress memAddr = { nullptr, baseAddrHash, offset };
    memAddr.value = createRegister(regVarnode, memAddr);
    memAddr.value->setLinearExpr(memAddr.value->getLinearExpr() + offset); // for ah/xmm registers
    return memAddr;
}

ircode::MemoryAddress IRcodeGenerator::getMemoryAddress(std::shared_ptr<ircode::Value> addrValue) const {
    const auto& addrExpr = addrValue->getLinearExpr();
    assert(!addrExpr.getTerms().empty());
    auto baseAddrValue = addrExpr.getTerms().front().value;
    assert(baseAddrValue->getSize() == 8 && "Invalid address size");
    return {
        addrValue,
        baseAddrValue->getHash(),
        addrExpr.getConstTermValue()
    };
}

std::list<IRcodeGenerator::VariableReadInfo> IRcodeGenerator::genReadRegisterVarnode(std::shared_ptr<pcode::RegisterVarnode> regVarnode) {
    auto memAddr = getRegisterMemoryAddress(regVarnode);
    auto readSize = regVarnode->getSize();
    auto readMask = utils::BitMask(readSize, 0);
    return genReadMemory(memAddr, readSize, readMask);
}


std::shared_ptr<ircode::Value> IRcodeGenerator::genReadVarnode(std::shared_ptr<pcode::Varnode> varnode) {
    if (auto regVarnode = std::dynamic_pointer_cast<pcode::RegisterVarnode>(varnode)) {
        auto varReadInfos = genReadRegisterVarnode(regVarnode);
        return joinVariables(varReadInfos, regVarnode->getSize());
    }
    else if (auto constVarnode = std::dynamic_pointer_cast<pcode::ConstantVarnode>(varnode)) {
        return createConstant(constVarnode);
    }
    
    throw std::runtime_error("Invalid varnode type");
}

void IRcodeGenerator::genOperation(std::unique_ptr<ircode::Operation> operation) {
    if (m_generationEnabled) {
        auto op = operation.get();
        m_genOperations.push_back(op);
        operation->setBlock(m_block);
        operation->getPcodeInstructions().insert(m_curInstr);
        operation->setOverwrittenVariables(m_overwrittenVariables);
        m_block->getOperations().push_back(std::move(operation));
        m_block->getFunction()->getProgram()->getEventPipe()->send(
            OperationAddedEvent(op));
    }
    m_overwrittenVariables.clear();
}

void IRcodeGenerator::genGenericOperation(const pcode::Instruction* instr, ircode::OperationId operationId, ircode::MemoryAddress& outputMemAddr) {
    // get input values
    std::shared_ptr<ircode::Value> inputVal1;
    std::shared_ptr<ircode::Value> inputVal2;
    if (auto input0 = instr->getInput0())
        inputVal1 = genReadVarnode(input0);
    if (auto input1 = instr->getInput1())
        inputVal2 = genReadVarnode(input1);
    if (!inputVal1)
        throw std::runtime_error("Invalid instruction");

    // exception for STORE instruction
    if (instr->getId() == pcode::InstructionId::STORE) {
        outputMemAddr = getMemoryAddress(inputVal1);
        inputVal1 = inputVal2;
        inputVal2 = nullptr;
    }
    
    // calculate hash
    ircode::Hash hash = 0;
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
    auto outputMemSpace = getCurMemSpace()->getSubspace(outputMemAddr.baseAddrHash);
    auto outputVar = createVariable(outputMemAddr, hash, outputMemAddr.value->getSize());
    genWriteMemory(outputMemSpace, outputVar);

    // generate operation
    if (inputVal2) {
        // calculate address as linear expression
        const auto& linearExprInp1 = inputVal1->getLinearExpr();
        const auto& linearExprInp2 = inputVal2->getLinearExpr();
        if (instr->getId() == pcode::InstructionId::INT_ADD) {
            outputVar->setLinearExpr(linearExprInp1 + linearExprInp2);
        } else if (instr->getId() == pcode::InstructionId::INT_MULT) {
            if (linearExprInp1.getTerms().empty() || linearExprInp2.getTerms().empty())
                outputVar->setLinearExpr(linearExprInp1 * linearExprInp2);
        }

        genOperation(std::make_unique<ircode::BinaryOperation>(operationId, inputVal1, inputVal2, outputVar));
    } else {
        genOperation(std::make_unique<ircode::UnaryOperation>(operationId, inputVal1, outputVar));
    }
}

std::shared_ptr<ircode::Variable> IRcodeGenerator::genLoadOperation(const ircode::MemoryAddress& memAddr, size_t loadSize) {
    ircode::Hash hash = 0;
    boost::hash_combine(hash, ircode::OperationId::LOAD);
    boost::hash_combine(hash, memAddr.value->getHash());

    auto regVariable = createVariable(MemoryAddress(), hash, loadSize); // MemoryAddress() vs memAddr?
    genOperation(std::make_unique<ircode::UnaryOperation>(ircode::OperationId::LOAD, memAddr.value, regVariable));
    return regVariable;
}

void IRcodeGenerator::genCallOperation(const pcode::Instruction* instr) {
    ircode::Hash hash = 0;
    boost::hash_combine(hash, ircode::OperationId::CALL);
    boost::hash_combine(hash, instr->getOffset().fullOffset); // TODO: make it like that everywhere?
    std::vector<std::shared_ptr<Value>> arguments;
    std::shared_ptr<Variable> output;
    auto input0 = instr->getInput0();
    auto destValue = genReadVarnode(input0);
    auto functions = m_block->getFunction()->getProgram()->getFunctionsByCallInstruction(instr);
    if (!functions.empty()) {
        auto function = functions.front();
        if (function->getFunctionSymbol()) {
            auto signature = function->getFunctionSymbol()->getSignature();
            auto platform = signature->getContext()->getPlatform();
            auto& parameters = signature->getParameters();
            auto& storages = signature->getStorages();
            // arguments
            for (size_t i = 0; i < parameters.size(); ++i) {
                auto param = parameters[i];
                auto paramSize = param->getDataType()->getSize();
                std::shared_ptr<pcode::RegisterVarnode> regVarnode;
                size_t offset = 0;
                bool isSpOrIp = false;
                for (auto& [storage, storageInfo] : storages) {
                    if (storageInfo.type == CallingConvention::StorageInfo::Parameter && storageInfo.paramIdx == i) {
                        auto regId = storage.registerId;
                        isSpOrIp = regId == sda::Register::InstructionPointerId || regId == sda::Register::StackPointerId;
                        auto reg = sda::Register::Create(
                            platform->getRegisterRepository().get(),
                            regId,
                            isSpOrIp ? platform->getPointerSize() : paramSize);
                        regVarnode = std::make_shared<pcode::RegisterVarnode>(reg);
                        offset = storage.offset;
                        break;
                    }
                }
                if (regVarnode) {
                    auto regId = regVarnode->getRegister().getRegId();
                    auto regValue = genReadVarnode(regVarnode);
                    if (isSpOrIp) {
                        auto offsetConst = createConstant(std::make_shared<pcode::ConstantVarnode>(offset, platform->getPointerSize(), false));
                        auto addrVarHash = hash;
                        boost::hash_combine(addrVarHash, i);
                        auto addrVar = createVariable(MemoryAddress(), hash, platform->getPointerSize());
                        addrVar->setLinearExpr(regValue->getLinearExpr() + offsetConst->getLinearExpr());
                        genOperation(std::make_unique<ircode::BinaryOperation>(ircode::OperationId::INT_ADD, regValue, offsetConst, addrVar));
                        auto memAddr = getMemoryAddress(addrVar);
                        utils::BitMask readMask(paramSize, 0);
                        auto varReadInfos = genReadMemory(memAddr, paramSize, readMask);
                        auto input = joinVariables(varReadInfos, paramSize);
                        arguments.push_back(input);
                    } else {
                        arguments.push_back(regValue);
                    }
                } else {
                    throw std::runtime_error("Parameter not found");
                }
            }
            // output
            {
                auto resultSize = std::max(signature->getReturnType()->getSize(), size_t(1));
                std::shared_ptr<pcode::RegisterVarnode> regVarnode;
                size_t offset = 0;
                for (auto& [storage, storageInfo] : storages) {
                    if (storageInfo.type == CallingConvention::StorageInfo::Return) {
                        auto reg = sda::Register::Create(
                            platform->getRegisterRepository().get(),
                            storage.registerId,
                            resultSize);
                        regVarnode = std::make_shared<pcode::RegisterVarnode>(reg);
                        offset = storage.offset;
                    }
                }
                if (regVarnode) {
                    auto outputMemAddr = getRegisterMemoryAddress(regVarnode);
                    auto memSpace = getCurMemSpace()->getSubspace(outputMemAddr.baseAddrHash);
                    output = createVariable(outputMemAddr, hash, resultSize);
                    genWriteMemory(memSpace, output);
                } else {
                    output = createVariable(MemoryAddress(), hash, resultSize);
                }
            }
        } else {
            output = createVariable(MemoryAddress(), hash, 1);
        }
    } else {
        output = createVariable(MemoryAddress(), hash, 1);
    }
    genOperation(std::make_unique<ircode::CallOperation>(destValue, arguments, output));
}

void IRcodeGenerator::handleConditionJumpOperation(const pcode::Instruction* instr) {
    if (auto conditionVarnode = instr->getInput1()) {
        m_block->getCondition() = genReadVarnode(conditionVarnode);
    }
}

std::shared_ptr<ircode::Constant> IRcodeGenerator::createConstant(std::shared_ptr<pcode::ConstantVarnode> constVarnode) const {
    auto hash = std::hash<size_t>()(constVarnode->getValue());
    boost::hash_combine(hash, ircode::Value::Constant);
    auto value = std::make_shared<ircode::Constant>(constVarnode, hash);
    value->setLinearExpr(value->getConstVarnode()->getValue());
    return value;
}

std::shared_ptr<ircode::Register> IRcodeGenerator::createRegister(std::shared_ptr<pcode::RegisterVarnode> regVarnode, const ircode::MemoryAddress& memAddr) const {
    auto hash = memAddr.baseAddrHash;
    boost::hash_combine(hash, memAddr.offset);
    boost::hash_combine(hash, regVarnode->getSize());
    boost::hash_combine(hash, ircode::Value::Register);
    auto value = std::make_shared<ircode::Register>(regVarnode, hash);
    value->setLinearExpr(ircode::LinearExpression(value));
    return value;
}

std::shared_ptr<ircode::Variable> IRcodeGenerator::createVariable(const ircode::MemoryAddress& memAddress, ircode::Hash hash, size_t size) {
    boost::hash_combine(hash, size);
    boost::hash_combine(hash, ircode::Value::Variable);
    auto varId = m_generationEnabled ? m_nextVarIdProvider() : 0;
    auto value = std::make_shared<ircode::Variable>(varId, memAddress, hash, size);
    value->setLinearExpr(ircode::LinearExpression(value));
    return value;
}

MemorySpace* IRcodeGenerator::getCurMemSpace() const {
    return m_block->getMemorySpace();
}
