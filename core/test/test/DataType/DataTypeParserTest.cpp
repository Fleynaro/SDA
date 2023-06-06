#include "SDA/Core/DataType/DataTypeParser.h"
#include "SDA/Core/DataType/DataTypePrinter.h"
#include "Test/Core/ContextFixture.h"
#include "Test/Core/Plaftorm/CallingConventionMock.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataTypeParserTest : public ContextFixture
{};

TEST_F(DataTypeParserTest, TypedefSample1) {
    auto expectedCode = "\
        ['test data type'] \
        dataType = typedef uint32_t \
    ";
    auto typedefDt = parseDataType(expectedCode);
    ASSERT_TRUE(cmpDataType(typedefDt, expectedCode, true));
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
    ASSERT_TRUE(cmpDataType(enumDt, expectedCode, true));
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
    ASSERT_TRUE(cmpDataType(structDt, expectedCode, true));
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
    ASSERT_TRUE(cmpDataType(signatureDt, expectedCode, true));
}