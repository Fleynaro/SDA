#pragma once
#include <gtest/gtest.h>
#include "Core/Context.h"
#include "Core/DataType/DataType.h"
#include "Core/Test/Plaftorm/PlatformMock.h"

namespace sda::test
{
    class ContextFixture : public ::testing::Test
    {
    protected:
        Context* context;

        void SetUp() override;
        
        void TearDown() override;

        PlatformMock* getPlatform() const;

        DataType* findDataType(const std::string& name) const;

        DataType* parseDataType(const std::string& text) const;

        DataType* newTestStruct() const;
    };
};