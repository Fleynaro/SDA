#include "Core/DataType/DataTypeParser.h"
#include "Core/DataType/DataTypePrinter.h"
#include "Core/Test/ContextFixture.h"
#include "Core/Test/Plaftorm/CallingConventionMock.h"
#include "Core/Test/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataTypeParserTest : public ContextFixture
{
protected:
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
    auto typedefDt = parseDataType(expectedCode);
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
    auto enumDt = parseDataType(expectedCode);
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
    auto structDt = parseDataType(expectedCode);
    ASSERT_TRUE(cmp(structDt, expectedCode));
}

TEST_F(DataTypeParserTest, SignatureSample1) {
    auto expectedCode = "\
        ['test data type'] \
        dataType = signature fastcall void( \
            uint32_t param1, \
            float param2 \
        ) \
    ";
    auto signatureDt = parseDataType(expectedCode);
    ASSERT_TRUE(cmp(signatureDt, expectedCode));
}