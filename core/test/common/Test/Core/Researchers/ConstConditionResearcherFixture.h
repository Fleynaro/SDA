#pragma once
#include "ResearcherFixture.h"
#include "SDA/Core/Researchers/ConstConditionResearcher.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ConstConditionSemanticsFixture : public SemanticsFixture
{
protected:
    std::unique_ptr<semantics::ConstConditionRepository> constCondRepo;

    void SetUp() override {
        IRcodeFixture::SetUp();
        constCondRepo = std::make_unique<semantics::ConstConditionRepository>(program);
        eventPipe->connect(constCondRepo->getEventPipe());
    }

    ::testing::AssertionResult cmpConditions(ircode::Function* function, const std::string& expectedCode) const {
        return CmpConditions(constCondRepo.get(), function, expectedCode);
    }

public:
    static ::testing::AssertionResult CmpConditions(semantics::ConstConditionRepository* constCondRepo, ircode::Function* function, const std::string& expectedCode) {
        std::stringstream ss;
        utils::AbstractPrinter printer;
        printer.setOutput(ss);
        for (size_t i = 0; i < 2; ++i)
            printer.startBlock();
        printer.newTabs();
        auto blockInfos = function->getFunctionGraph()->getBlocks(true);
        for (auto& [pcodeBlock, level] : blockInfos) {
            auto block = function->toBlock(pcodeBlock);
            auto conditions = constCondRepo->findConditions(block);
            if (!conditions.empty()) {
                ss << "Block " << block->getName() << ":";
                printer.startBlock();
                for (auto& cond : conditions) {
                    printer.newLine();
                    ss << cond.var->getName();
                    ss << (cond.type == semantics::ConstantCondition::EQUAL ? " == " : " != ");
                    ss << cond.value;
                }
                printer.endBlock();
                printer.newLine();
            }
        }
        return Compare(ss.str(), expectedCode);
    }
};