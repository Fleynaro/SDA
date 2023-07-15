#pragma once
#include "ResearcherFixture.h"
#include "SDA/Core/Researchers/DataFlowResearcher.h"
#include "SDA/Core/Utils/String.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataFlowResearcherFixture : public ResearcherFixture
{
protected:
    std::unique_ptr<researcher::DataFlowRepository> dataFlowRepo;
    std::unique_ptr<researcher::DataFlowCollector> dataFlowCollector;

    void SetUp() override;

    ::testing::AssertionResult cmpDataFlow(ircode::Function* function, const std::string& expectedCode) const;

    static std::string GetNodeName(researcher::DataFlowNode* node, ircode::Function* function = nullptr);

public:
    static ::testing::AssertionResult CmpDataFlow(researcher::DataFlowRepository* dataFlowRepo, ircode::Function* function, const std::string& expectedCode);
};