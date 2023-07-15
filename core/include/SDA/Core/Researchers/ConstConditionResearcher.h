#pragma once
#include "SDA/Core/IRcode/IRcodeProgram.h"
#include "SDA/Core/IRcode/IRcodeEvents.h"

namespace sda::semantics
{
    struct ConstantCondition {
        enum {
            EQUAL,
            NOT_EQUAL
        } type = EQUAL;
        std::shared_ptr<ircode::Variable> var;
        size_t value = 0;
    };

    class ConstConditionRepository
    {
        struct Condition {
            std::shared_ptr<ircode::Variable> var;
            size_t value = 0;
            ircode::Block* block = nullptr;
            utils::BitSet thenBlocks;
            utils::BitSet elseBlocks;
            bool inverted = false;
        };
        struct FunctionInfo {
            std::list<Condition> conditions;
        };
        std::map<ircode::Function*, FunctionInfo> m_functionInfos;

        class IRcodeEventHandler
        {
            ConstConditionRepository* m_repo;

            void handleFunctionDecompiled(const ircode::FunctionDecompiledEvent& event) {
                m_repo->research(event.function, event.blocks);
            }

            void handleBlockRemovedEvent(const ircode::BlockRemovedEvent& event) {
                auto functionInfo = m_repo->getFunctionInfo(event.block->getFunction());
                m_repo->removeConditions(functionInfo, event.block);
            }
        public:
            IRcodeEventHandler(ConstConditionRepository* repo) : m_repo(repo) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleFunctionDecompiled);
                pipe->subscribeMethod(this, &IRcodeEventHandler::handleBlockRemovedEvent);
                return pipe;
            }
        };
        IRcodeEventHandler m_ircodeEventHandler;
    public:
        ConstConditionRepository(ircode::Program* program)
            : m_ircodeEventHandler(this)
        {}

        std::shared_ptr<EventPipe> getEventPipe() {
            return m_ircodeEventHandler.getEventPipe();
        }

        std::list<ConstantCondition> findConditions(ircode::Block* block) {
            std::list<ConstantCondition> conditions;
            if(auto funcInfo = getFunctionInfo(block->getFunction())) {
                for (auto cond : funcInfo->conditions) {
                    bool isInThenScope = cond.thenBlocks.get(block->getIndex());
                    bool isInElseScope = cond.elseBlocks.get(block->getIndex());
                    if (isInThenScope ^ isInElseScope) {
                        conditions.push_back({
                            (isInThenScope ^ cond.inverted) ? ConstantCondition::EQUAL : ConstantCondition::NOT_EQUAL,
                            cond.var,
                            cond.value
                        });
                    }
                }
            }
            return conditions;
        }

    private:
        void research(ircode::Function* function, const std::list<ircode::Block*>& blocks) {
            auto functionInfo = getFunctionInfo(function);
            for (auto block : blocks) {
                removeConditions(functionInfo, block);
                if (auto condVar = std::dynamic_pointer_cast<ircode::Variable>(block->getCondition())) {
                    auto op = condVar->getSourceOperation();
                    if (op->getId() == ircode::OperationId::INT_EQUAL || op->getId() == ircode::OperationId::INT_NOTEQUAL) {
                        if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(op)) {
                            if (auto inputVar1 = std::dynamic_pointer_cast<ircode::Variable>(binaryOp->getInput1())) {
                                if (auto inputVar2 = std::dynamic_pointer_cast<ircode::Constant>(binaryOp->getInput2())) {
                                    auto value = inputVar2->getConstVarnode()->getValue();
                                    auto thenBlocks = passBlocks(block, block->getFarNextBlock());
                                    auto elseBlocks = passBlocks(block, block->getNearNextBlock());
                                    addCondition(functionInfo, {
                                        inputVar1,
                                        value,
                                        block,
                                        thenBlocks,
                                        elseBlocks,
                                        op->getId() == ircode::OperationId::INT_NOTEQUAL
                                    });
                                }
                            }
                        }
                    }
                }
            }
        }

        utils::BitSet passBlocks(ircode::Block* condBlock, ircode::Block* block) {
            utils::BitSet result;
            block->passDescendants([&result, condBlock](ircode::Block* block, bool& goNextBlocks) {
                if (block == condBlock) return;
                result.set(block->getIndex(), true);
                goNextBlocks = true;
            });
            return result;
        }

        FunctionInfo* getFunctionInfo(ircode::Function* function) {
            auto it = m_functionInfos.find(function);
            if (it == m_functionInfos.end()) {
                it = m_functionInfos.insert({ function, {} }).first;
            }
            return &it->second;
        }

        void addCondition(FunctionInfo* functionInfo, const Condition& condition) {
            functionInfo->conditions.push_back(condition);
        }

        void removeConditions(FunctionInfo* functionInfo, ircode::Block* block) {
            functionInfo->conditions.remove_if([block](const Condition& cond) {
                return cond.block == block;
            });
        }
    };
};