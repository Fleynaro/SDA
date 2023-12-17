#include "SDA/Core/DataType/DataTypeParser.h"
#include "SDA/Core/DataType/DataTypePrinter.h"
#include "Test/Core/ContextFixture.h"
#include "Test/Core/Plaftorm/CallingConventionMock.h"
#include <boost/uuid/uuid_io.hpp>

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class DataTypeParserTest : public ContextFixture
{};

TEST_F(DataTypeParserTest, TypedefSample1) {
    auto expectedCode = "\
        // test data type \n\
        dataType = typedef uint32_t \
    ";
    auto typedefDt = parseDataType(expectedCode);
    ASSERT_TRUE(cmpDataType(typedefDt, expectedCode, true));
}

TEST_F(DataTypeParserTest, TypedefSample2) {
    auto expectedCode = "\
        dataType = typedef uint32_t \
    ";
    auto typedefDt = parseDataType(expectedCode);
    ASSERT_TRUE(cmpDataType(typedefDt, expectedCode, true));
    // rename to myTypeDef and change reference type to uint64_t
    auto expectedCode2 = "\
        @id '" + boost::uuids::to_string(typedefDt->getId()) + "' \n\
        myTypeDef = typedef uint64_t \
    ";
    parseDataType(expectedCode2);
    ASSERT_TRUE(cmpDataType(typedefDt, expectedCode2, true, true));
}

TEST_F(DataTypeParserTest, EnumSample1) {
    auto expectedCode = "\
        // test data type \n\
        // this contains A, B, C values \n\
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
        dataType = struct { \
            uint32_t a, \
            // some comment \n\
            float b, \
            uint64_t c = 0x100, \
            int64_t d \
        } \
    ";
    auto structDt = parseDataType(expectedCode);
    ASSERT_TRUE(cmpDataType(structDt, expectedCode, true));
}

TEST_F(DataTypeParserTest, StructureSample2) {
    auto expectedCode = "\
        Scalar = typedef float \
        \
        Vector3D = struct { \
            Scalar x, \
            Scalar y, \
            Scalar z \
        } \
        \
        Player = struct { \
            Vector3D pos \
        } \
    ";
    auto dataTypes = parseDataTypes(expectedCode);
    ASSERT_TRUE(cmpDataTypes(dataTypes, expectedCode));
}

TEST_F(DataTypeParserTest, StructureSample3) {
    auto expectedCode = "\
        Player = struct { \
            Player* friend \
        } \
    ";
    auto dataTypes = parseDataTypes(expectedCode);
    ASSERT_TRUE(cmpDataTypes(dataTypes, expectedCode));
}

TEST_F(DataTypeParserTest, StructureSample4) {
    auto expectedCode = "\
        Player = struct \
        \
        Vehicle = struct { \
            Player* driver \
        } \
        \
        Player = struct { \
            Vehicle* sitInVehicle \
        } \
    ";
    auto dataTypes = parseDataTypes(expectedCode);
    ASSERT_TRUE(cmpDataTypes(dataTypes, expectedCode));
}

TEST_F(DataTypeParserTest, SignatureSample1) {
    auto expectedCode = "\
        dataType = signature fastcall void( \
            uint32_t param1, \
            float param2 \
        ) \
    ";
    auto signatureDt = parseDataType(expectedCode);
    ASSERT_TRUE(cmpDataType(signatureDt, expectedCode, true));
}

TEST_F(DataTypeParserTest, SignatureSample2) {
    auto expectedCode = "\
        dataType = signature fastcall void( \
            uint32_t param1 \
        ) \
    ";
    auto signatureDt = parseDataType(expectedCode);
    ASSERT_TRUE(cmpDataType(signatureDt, expectedCode, true));
    // add param2 to the signature
    auto expectedCode2 = "\
        @id '" + boost::uuids::to_string(signatureDt->getId()) + "' \n\
        dataType = signature fastcall void( \
            uint32_t param1, \
            float param2 \
        ) \
    ";
    parseDataType(expectedCode2);
    ASSERT_TRUE(cmpDataType(signatureDt, expectedCode2, true, true));
    // remove param1 from the signature
    auto expectedCode3 = "\
        @id '" + boost::uuids::to_string(signatureDt->getId()) + "' \n\
        dataType = signature fastcall void( \
            float param2 \
        ) \
    ";
    parseDataType(expectedCode3);
    ASSERT_TRUE(cmpDataType(signatureDt, expectedCode3, true, true));
}