#pragma once
#include "ResearcherFixture.h"
#include "SDA/Core/Researchers/ConstConditionResearcher.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ConstConditionResearcherFixture : public ResearcherFixture
{
protected:
    std::unique_ptr<researcher::ConstConditionRepository> constCondRepo;

    void SetUp() override {
        IRcodeFixture::SetUp();
        constCondRepo = std::make_unique<researcher::ConstConditionRepository>(program);
        eventPipe->connect(constCondRepo->getEventPipe());
    }

    ::testing::AssertionResult cmpConditions(ircode::Function* function, const std::string& expectedCode) const {
        return Compare(PrintConditionsForFunction(constCondRepo.get(), function), expectedCode);
    }

public:
    static ::testing::AssertionResult CmpConditions(researcher::ConstConditionRepository* constCondRepo, ircode::Function* function, const std::string& expectedCode) {
        return Compare(PrintConditionsForFunction(constCondRepo, function), expectedCode);
    }
};