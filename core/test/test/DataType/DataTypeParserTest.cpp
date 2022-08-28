#include "Core/DataType/DataTypeParser.h"
#include "Core/DataType/DataTypePrinter.h"
#include "Core/Test/ContextFixture.h"
#include "Core/Test/Plaftorm/CallingConventionMock.h"
#include "Core/Test/Utils.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataTypeParserTest : public ContextFixture
{
protected:
    std::map<std::string, DataType*> parse(const std::string& text) {
        return DataTypeParser::Parse(text, context);
    }

    DataType* parseSingle(const std::string& text) {
        return DataTypeParser::ParseSingle(text, context, true);
    }

    ::testing::AssertionResult cmp(DataType* dataType, const std::string& expectedCode) const {
        auto actualCode = DataTypePrinter::Print(dataType, context, true);
        return Compare(actualCode, expectedCode);
    }
};

TEST_F(DataTypeParserTest, TypedefSample1) {
    auto expectedCode = "\
        ['test data type'] \
        dataType = typedef uint32_t \
    ";
    auto typedefDt = parseSingle(expectedCode);
    ASSERT_TRUE(cmp(typedefDt, expectedCode));
}

TEST_F(DataTypeParserTest, EnumSample1) {
    auto expectedCode = "\
        ['test data type'] \
        dataType = enum { \
            A, \
            B = 10, \
            C \
        } \
    ";
    auto enumDt = parseSingle(expectedCode);
    ASSERT_TRUE(cmp(enumDt, expectedCode));
}

TEST_F(DataTypeParserTest, StructureSample1) {
    auto expectedCode = "\
        ['test data type'] \
        dataType = struct { \
            uint32_t a, \
            float b, \
            uint64_t c = 0x100, \
            int64_t d \
        } \
    ";
    auto structDt = parseSingle(expectedCode);
    ASSERT_TRUE(cmp(structDt, expectedCode));
}

TEST_F(DataTypeParserTest, SignatureSample1) {
    auto expectedCode = "\
        ['test data type'] \
        dataType = signature testcall void( \
            uint32_t param1, \
            float param2 \
        ) \
    ";
    auto testCallConv = std::make_shared<CallingConventionMock>();
    EXPECT_CALL(*testCallConv, getName())
        .WillRepeatedly(Return("testcall"));
    std::list<std::shared_ptr<CallingConvention>> callingConventions = {testCallConv};
    EXPECT_CALL(*getPlatform(), getCallingConventions())
        .WillOnce(ReturnRef(callingConventions));
    auto signatureDt = parseSingle(expectedCode);
    ASSERT_TRUE(cmp(signatureDt, expectedCode));
}