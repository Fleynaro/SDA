#pragma once
#include "SemanticsFixture.h"
#include "SDA/Core/Semantics/DataFlowSemantics.h"
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

    static std::string GetVariableName(std::shared_ptr<ircode::Variable> var, ircode::Function* function = nullptr);

public:
    static ::testing::AssertionResult CmpDataFlow(semantics::DataFlowRepository* dataFlowRepo, ircode::Function* function, const std::string& expectedCode);
};