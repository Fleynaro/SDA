#pragma once
#include "ResearcherFixture.h"
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
    std::set<std::string> showStructures;

    void SetUp() override {
        ResearcherFixture::SetUp();
        constCondRepo = std::make_unique<researcher::ConstConditionRepository>(program);
        structureRepo = std::make_unique<researcher::StructureRepository>(eventPipe);
        dataFlowRepo = std::make_unique<researcher::DataFlowRepository>(eventPipe);
        dataFlowCollector = std::make_unique<researcher::DataFlowCollector>(
            program,
            context->getPlatform(),
            dataFlowRepo.get());
        structureResearcher = std::make_unique<researcher::StructureResearcher>(
            program,
            context->getPlatform(),
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
        for (auto rootStructure : sortByName(rootStructures)) {
            gatherAllChildStructures(rootStructure, allStructures);
        }
        for (auto structure : sortByName(structureRepo->getAllStructures())) {
            if (std::find(allStructures.begin(), allStructures.end(), structure) != allStructures.end()) {
                continue;
            }
            allStructures.push_back(structure);
        }
        std::stringstream ss;
        bool isFirstPrinted = false;
        for (auto structure : allStructures) {
            auto forciblyShown = showStructures.find(structure->name) != showStructures.end();
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
            printStructure(ss, structure);
        }
        return Compare(ss.str(), expectedCode);
    }

    void gatherAllChildStructures(researcher::Structure* rootStructure, std::list<researcher::Structure*>& result) const {
        std::list<researcher::Structure*> structuresToVisit;
        structuresToVisit.push_back(rootStructure);
        while (!structuresToVisit.empty()) {
            auto structure = structuresToVisit.front();
            structuresToVisit.pop_front();
            result.push_back(structure);
            for (auto child : sortByName(structure->childs)) {
                if (std::find(result.begin(), result.end(), child) != result.end()) {
                    continue;
                }
                structuresToVisit.push_back(child);
            }
        }
    }

    void printStructure(std::stringstream& ss, researcher::Structure* structure) const {
        ss << "struct " << structure->name << " ";
        if (!structure->parents.empty()) {
            ss << ": ";
            auto parents = sortByName(structure->parents);
            for (auto parent : parents) {
                ss << parent->name;
                if (parent != parents.back()) {
                    ss << ", ";
                }
            }
            ss << " ";
        }
        ss << "{" << std::endl;
        std::set<size_t> fieldOffsets;
        for (auto& [offset, _] : structure->fields) {
            fieldOffsets.insert(offset);
        }
        researcher::ConstantSet labelSet;
        labelSet.merge(structure->conditions);
        labelSet.merge(structure->constants, true);
        for (auto& [offset, _] : labelSet.values()) {
            fieldOffsets.insert(offset);
        }
        for (auto offset : fieldOffsets) {
            std::string sep;
            std::stringstream fieldValues;
            // structures
            auto it = structure->fields.find(offset);
            if (it != structure->fields.end()) {      
                auto childs = sortByName(it->second->childs);
                for (auto child : childs) {
                    fieldValues << sep << child->name;
                    sep = ", ";
                }
            }
            // labels
            auto it2 = labelSet.values().find(offset);
            if (it2 != labelSet.values().end()) {
                for (auto value : it2->second) {
                    fieldValues << sep << "0x" << utils::ToHex(value);
                    sep = ", ";
                }
            }
            ss << "    " << "0x" << utils::ToHex(offset) << ": " << fieldValues.str() << std::endl;
        }
        ss << "}";
    }

    std::list<researcher::Structure*> sortByName(std::list<researcher::Structure*> structures) const {
        structures.sort([](researcher::Structure* a, researcher::Structure* b) {
            return a->name < b->name;
        });
        return structures;
    }

    std::list<researcher::Structure*> sortByName(std::set<researcher::Structure*> structures) const {
        std::list<researcher::Structure*> result;
        for (auto structure : structures) {
            result.push_back(structure);
        }
        return sortByName(result);
    }
};