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
                for (auto block : event.blocks) { 
                    if (auto condVar = std::dynamic_pointer_cast<ircode::Variable>(block->getCondition())) {
                        auto op = condVar->getSourceOperation();
                        if (op->getId() == ircode::OperationId::INT_EQUAL || op->getId() == ircode::OperationId::INT_NOTEQUAL) {
                            if (auto binaryOp = dynamic_cast<const ircode::BinaryOperation*>(op)) {
                                if (auto inputVar1 = std::dynamic_pointer_cast<ircode::Variable>(binaryOp->getInput1())) {
                                    if (auto inputVar2 = std::dynamic_pointer_cast<ircode::Constant>(binaryOp->getInput2())) {
                                        auto value = inputVar2->getConstVarnode()->getValue();
                                        m_repo->addCondition(event.function, {
                                            inputVar1,
                                            value,
                                            block,
                                            op->getId() == ircode::OperationId::INT_NOTEQUAL
                                        });
                                    }
                                }
                            }
                        }
                    }
                }
            }
        public:
            IRcodeEventHandler(ConstConditionRepository* repo) : m_repo(repo) {}

            std::shared_ptr<EventPipe> getEventPipe() {
                auto pipe = EventPipe::New();
                pipe->handleMethod(this, &IRcodeEventHandler::handleFunctionDecompiled);
                return pipe;
            }
        };
        IRcodeEventHandler m_ircodeEventHandler;
    public:
        ConstConditionRepository(ircode::Program* program)
            : m_ircodeEventHandler(this)
        {
            program->getEventPipe()->connect(m_ircodeEventHandler.getEventPipe());
        }

        std::list<ConstantCondition> findConditions(ircode::Block* block) {
            std::list<ConstantCondition> conditions;
            auto dominants = block->getPcodeBlock()->getDominantBlocksSet();
            if(auto funcInfo = getFunctionInfo(block->getFunction())) {
                for (auto cond : funcInfo->conditions) {
                    bool isFar = dominants.get(cond.block->getFarNextBlock()->getIndex());
                    bool isNear = dominants.get(cond.block->getNearNextBlock()->getIndex());
                    if (isFar ^ isNear) {
                        conditions.push_back({
                            (isFar ^ cond.inverted) ? ConstantCondition::EQUAL : ConstantCondition::NOT_EQUAL,
                            cond.var,
                            cond.value
                        });
                    }
                }
            }
            return conditions;
        }

    private:
        FunctionInfo* getFunctionInfo(ircode::Function* function) {
            auto it = m_functionInfos.find(function);
            if (it == m_functionInfos.end()) {
                return nullptr;
            }
            return &it->second;
        }

        void addCondition(ircode::Function* function, const Condition& condition) {
            auto functionInfo = getFunctionInfo(function);
            if (!functionInfo) {
                m_functionInfos[function] = {};
                functionInfo = getFunctionInfo(function);
            }
            functionInfo->conditions.push_back(condition);
        }
    };
};