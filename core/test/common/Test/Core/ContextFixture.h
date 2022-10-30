#pragma once
#include <gtest/gtest.h>
#include "SDA/Core/ContextInclude.h"
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"

namespace sda::test
{
    class ContextFixture : public ::testing::Test
    {
        Platform* platform = nullptr;
        std::list<Context*> createdContexts;
    protected:
        Context* context;

        void SetUp() override;
        
        void TearDown() override;

        Context* newContext();

        DataType* findDataType(const std::string& name) const;

        DataType* parseDataType(const std::string& text, bool withName = true) const;

        DataType* newTestStruct() const;

        SymbolTable* parseSymbolTable(const std::string& text, bool withName = true) const;
    };
};