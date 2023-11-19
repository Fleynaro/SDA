#pragma once
#include "StructureResearcherFixture.h"
#include "SDA/Core/Researchers/ClassResearcher.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ClassResearcherFixture : public StructureResearcherFixture
{
protected:
    std::unique_ptr<researcher::ClassRepository> classRepo;
    std::unique_ptr<researcher::ClassResearcher> classResearcher;

    void SetUp() override {
        StructureResearcherFixture::SetUp();
        classRepo = std::make_unique<researcher::ClassRepository>(eventPipe);
        classResearcher = std::make_unique<researcher::ClassResearcher>(
            program,
            context->getPlatform(),
            classRepo.get(),
            structureRepo.get());
        eventPipe->connect(classResearcher->getEventPipe());
    }

    ::testing::AssertionResult cmpStructureInfos(const std::string& expectedCode) const {
        std::stringstream ss;
        for (auto structure : sortByName(structureRepo->getAllStructures())) {
            auto info = classRepo->getStructureInfo(structure);
            auto values = info->getLabelSet().values();
            if (values.empty()) continue;
            ss << structure->name << std::endl;
            for (auto& [offset, valuesAtOffset] : values) {
                std::string sep;
                std::stringstream fieldValues;
                for (auto value : valuesAtOffset) {
                    fieldValues << sep << "0x" << utils::ToHex(value);
                    sep = ", ";
                }
                ss << "    0x" << utils::ToHex(offset) << ": " << fieldValues.str() << std::endl;
            }
        }
        return Compare(ss.str(), expectedCode);
    }

    ::testing::AssertionResult cmpFieldStructureGroups(const std::string& expectedCode) const {
        std::list<std::string> strings;
        for (auto& group : classRepo->getAllFieldStructureGroups()) {
            if (group.getStructures().size() == 1) continue;
            std::string sep;
            std::stringstream ss;
            for (auto structure : sortByName(group.getStructures())) {
                ss << sep << structure->name;
                sep = ", ";
            }
            strings.push_back(ss.str());
        }
        strings.sort();
        std::stringstream ss;
        for (auto& str : strings) {
            ss << "{ " << str << " }" << std::endl;
        }
        return Compare(ss.str(), expectedCode);
    }
};