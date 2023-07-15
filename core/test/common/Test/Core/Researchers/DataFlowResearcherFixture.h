#pragma once
#include "ResearcherFixture.h"
#include "SDA/Core/Researchers/DataFlowResearcher.h"
#include "SDA/Core/Utils/IOManip.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataFlowSemanticsFixture : public SemanticsFixture
{
protected:
    std::unique_ptr<semantics::DataFlowRepository> dataFlowRepo;
    std::unique_ptr<semantics::DataFlowCollector> dataFlowCollector;

    void SetUp() override;

    ::testing::AssertionResult cmpDataFlow(ircode::Function* function, const std::string& expectedCode) const;

    static std::string GetNodeName(semantics::DataFlowNode* node, ircode::Function* function = nullptr);

public:
    static ::testing::AssertionResult CmpDataFlow(semantics::DataFlowRepository* dataFlowRepo, ircode::Function* function, const std::string& expectedCode);
};