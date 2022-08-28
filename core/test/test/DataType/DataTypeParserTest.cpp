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

    std::string print(DataType* dataType) const {
        return DataTypePrinter::Print(dataType, context, true);
    }
};

TEST_F(DataTypeParserTest, TypedefSample1) {
    auto expectedCode = "\
        ['test data type'] \
        dataType = typedef uint32_t \
    ";
    auto typedefDt = parseSingle(expectedCode);
    auto actualCode = print(typedefDt);
    ASSERT_TRUE(Compare(actualCode, expectedCode));
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
    auto actualCode = print(enumDt);
    ASSERT_TRUE(Compare(actualCode, expectedCode));
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
    auto actualCode = print(structDt);
    ASSERT_TRUE(Compare(actualCode, expectedCode));
}

TEST_F(DataTypeParserTest, SignatureSample1) {
    auto expectedCode = "\
        ['test data type'] \
        dataType = signature fastcall void( \
            uint32_t param1, \
            float param2 \
        ) \
    ";
    auto callingConvention = std::make_shared<CallingConventionMock>();
    EXPECT_CALL(*callingConvention, getName())
        .WillRepeatedly(Return("fastcall"));
    std::list<std::shared_ptr<CallingConvention>> callingConventions = {callingConvention};
    EXPECT_CALL(*getPlatform(), getCallingConventions())
        .WillOnce(ReturnRef(callingConventions));
    auto signatureDt = parseSingle(expectedCode);
    auto actualCode = print(signatureDt);
    ASSERT_TRUE(Compare(actualCode, expectedCode));
}