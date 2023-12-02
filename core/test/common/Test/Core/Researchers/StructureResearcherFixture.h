#pragma once
#include "DataFlowResearcherFixture.h"
#include "ConstConditionResearcherFixture.h"
#include "SDA/Core/Researchers/StructureResearcher.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class StructureResearcherFixture : public ResearcherFixture
{
protected:
    std::unique_ptr<researcher::ConstConditionRepository> constCondRepo;
    std::unique_ptr<researcher::DataFlowRepository> dataFlowRepo;
    std::unique_ptr<researcher::DataFlowCollector> dataFlowCollector;
    std::unique_ptr<researcher::StructureRepository> structureRepo;
    std::unique_ptr<researcher::StructureResearcher> structureResearcher;
    std::set<std::string> showHiddenStructures;

    void SetUp() override {
        ResearcherFixture::SetUp();
        constCondRepo = std::make_unique<researcher::ConstConditionRepository>(program);
        structureRepo = std::make_unique<researcher::StructureRepository>(eventPipe);
        dataFlowRepo = std::make_unique<researcher::DataFlowRepository>(eventPipe);
        dataFlowCollector = std::make_unique<researcher::DataFlowCollector>(
            program,
            dataFlowRepo.get());
        structureResearcher = std::make_unique<researcher::StructureResearcher>(
            program,
            structureRepo.get(),
            dataFlowRepo.get(),
            constCondRepo.get());
        eventPipe->connect(constCondRepo->getEventPipe());
        eventPipe->connect(dataFlowCollector->getEventPipe());
        eventPipe->connect(structureResearcher->getEventPipe());
    }

    ::testing::AssertionResult cmpStructures(const std::string& expectedCode) const {
        std::list<researcher::Structure*> allStructures;
        auto rootStructures = structureRepo->getRootStructures();
        for (auto rootStructure : SortStructuresByName(rootStructures)) {
            GatherAllChildStructures(rootStructure, allStructures);
        }
        for (auto structure : SortStructuresByName(structureRepo->getAllStructures())) {
            if (std::find(allStructures.begin(), allStructures.end(), structure) != allStructures.end()) {
                continue;
            }
            allStructures.push_back(structure);
        }
        std::stringstream ss;
        bool isFirstPrinted = false;
        for (auto structure : allStructures) {
            auto forciblyShown = showHiddenStructures.find(structure->name) != showHiddenStructures.end();
            if (structure->fields.empty() &&
                structure->conditions.values().empty() && structure->constants.values().empty() &&
                !forciblyShown)
                continue;
            if (isFirstPrinted) {
                ss << std::endl << std::endl;
            } else {
                isFirstPrinted = true;
            }
            if (forciblyShown) {
                ss << "// forcibly shown" << std::endl;
            }
            PrintStructure(ss, structure);
        }
        return Compare(ss.str(), expectedCode);
    }

    ::testing::AssertionResult cmpConditions(ircode::Function* function, const std::string& expectedCode) const {
        return ConstConditionResearcherFixture::CmpConditions(constCondRepo.get(), function, expectedCode);
    }

    ::testing::AssertionResult cmpDataFlow(ircode::Function* function, const std::string& expectedCode) const {
       return DataFlowResearcherFixture::CmpDataFlow(dataFlowRepo.get(), function, expectedCode);
    }
};