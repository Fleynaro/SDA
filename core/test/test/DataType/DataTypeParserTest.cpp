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
        return DataTypeParser::ParseSingle(text, context);
    }
};

TEST_F(DataTypeParserTest, TypedefSample1) {
    auto typedefDt = parseSingle("\
        typedef uint32_t \
    ");

    auto str = DataTypePrinter::Print(typedefDt, context);
}

TEST_F(DataTypeParserTest, EnumSample1) {
    auto enumDt = parseSingle("\
        enum { \
            A, \
            B = 10, \
            C \
        } \
    ");
}

TEST_F(DataTypeParserTest, StructureSample1) {
    auto structDt = parseSingle("\
        struct { \
            uint32_t a, \
            float b, \
            uint64_t c = 0x100, \
            int64_t d = 20000 \
        } \
    ");
}

TEST_F(DataTypeParserTest, SignatureSample1) {
    auto callingConvention = std::make_shared<CallingConventionMock>();
    EXPECT_CALL(*callingConvention, getName())
        .WillOnce(Return("fastcall"));
    std::list<std::shared_ptr<CallingConvention>> callingConventions = {callingConvention};
    EXPECT_CALL(*getPlatform(), getCallingConventions())
        .WillOnce(ReturnRef(callingConventions));

    auto signatureDt = parseSingle("\
        signature fastcall void( \
            uint32_t param1, \
            float param2 \
        ) \
    ");
}