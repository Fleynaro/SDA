#include "SDA/Core/Image/AddressSpace.h"
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/DataType/VoidDataType.h"
#include "SDA/Core/DataType/PointerDataType.h"
#include "SDA/Core/DataType/ArrayDataType.h"
#include "SDA/Core/DataType/ScalarDataType.h"
#include "SDA/Core/DataType/TypedefDataType.h"
#include "SDA/Core/DataType/EnumDataType.h"
#include "SDA/Core/DataType/StructureDataType.h"
#include "SDA/Core/DataType/SignatureDataType.h"
#include "SDA/Core/DataType/DataTypePrinter.h"
#include "SDA/Core/Symbol/VariableSymbol.h"
#include "SDA/Core/Symbol/FunctionSymbol.h"
#include "SDA/Core/Symbol/FunctionParameterSymbol.h"
#include "SDA/Core/Symbol/StructureFieldSymbol.h"
#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/SymbolTable/OptimizedSymbolTable.h"
#include "SDA/Core/SymbolTable/SymbolTablePrinter.h"
#include "Test/Core/Image/ImageRWMock.h"
#include "Test/Core/Image/ImageAnalyserMock.h"
#include "Test/Core/Plaftorm/CallingConventionMock.h"
#include "Test/Core/ContextFixture.h"
#include "Test/Core/Utils/TestAssertion.h"
#include "SDA/Platform/X86/CallingConvention.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class SerializationTest : public ContextFixture
{
protected:
    ::testing::AssertionResult cmp(utils::ISerializable* object1, utils::ISerializable* object2) {
        return CompareDeeply(object1, object2, ContextObjectExcludeFields);
    }

    ::testing::AssertionResult cmp(DataType* dataType, const std::string& expectedCode) const {
        return cmpDataType(dataType, expectedCode, true);
    }

    ::testing::AssertionResult cmp(SymbolTable* symbolTable, const std::string& expectedCode) const {
        auto actualCode = SymbolTablePrinter::Print(symbolTable, context);
        return Compare(actualCode, expectedCode);
    }
};

TEST_F(SerializationTest, AddressSpace) {
    auto addressSpace1 = new AddressSpace(context, nullptr, "test");
    auto imageAnalyser = std::make_shared<ImageAnalyserMock>();
    addressSpace1->setImages({
        new Image(context, std::make_unique<ImageRWMock>(), imageAnalyser),
        new Image(context, std::make_unique<ImageRWMock>(), imageAnalyser)
    });
    boost::json::object data;
    addressSpace1->serialize(data);

    auto addressSpace2 = new AddressSpace(context);
    addressSpace2->deserialize(data);

    ASSERT_TRUE(cmp(addressSpace1, addressSpace2));
    ASSERT_STREQ(addressSpace2->getName().c_str(), "test");
}

TEST_F(SerializationTest, Image) {
    auto imageAnalyser = std::make_shared<ImageAnalyserMock>();
    auto imageRW1 = std::make_unique<ImageRWMock>();
    auto globalSymbolTable = new StandartSymbolTable(context);
    auto image1 = new Image(context, std::move(imageRW1), imageAnalyser, nullptr, "test", globalSymbolTable);
    boost::json::object data;
    image1->serialize(data);

    auto imageRW2 = std::make_unique<ImageRWMock>();
    auto image2 = new Image(context, std::move(imageRW2), imageAnalyser);
    image2->deserialize(data);

    ASSERT_TRUE(cmp(image1, image2));
    ASSERT_STREQ(image2->getName().c_str(), "test");
}

TEST_F(SerializationTest, VoidDataType) {
    auto dataType1 = new VoidDataType(context);
    boost::json::object data;
    dataType1->serialize(data);

    auto dataType2 = new VoidDataType(context);
    dataType2->deserialize(data);

    ASSERT_TRUE(cmp(dataType1, dataType2));
    ASSERT_STREQ(dataType2->getName().c_str(), "void");
}

TEST_F(SerializationTest, PointerDataType) {
    auto pointedDt = findDataType("uint32_t");
    auto dataType1 = new PointerDataType(context, nullptr, pointedDt);
    boost::json::object data;
    dataType1->serialize(data);

    auto dataType2 = new PointerDataType(context);
    dataType2->deserialize(data);

    ASSERT_TRUE(cmp(dataType1, dataType2));
    ASSERT_STREQ(dataType2->getName().c_str(), "uint32_t*");
}

TEST_F(SerializationTest, ArrayDataType) {
    auto elementDt = findDataType("uint32_t");
    auto dataType1 = new ArrayDataType(context, nullptr, elementDt, {2, 4, 10});
    boost::json::object data;
    dataType1->serialize(data);

    auto dataType2 = new ArrayDataType(context);
    dataType2->deserialize(data);

    ASSERT_TRUE(cmp(dataType1, dataType2));
    ASSERT_STREQ(dataType2->getName().c_str(), "uint32_t[2][4][10]");
}

TEST_F(SerializationTest, ScalarDataType) {
    auto dataType1 = new ScalarDataType(context, nullptr, "dataType", ScalarType::SignedInt, 4);
    boost::json::object data;
    dataType1->serialize(data);

    auto dataType2 = new ScalarDataType(context);
    dataType2->deserialize(data);

    ASSERT_TRUE(cmp(dataType1, dataType2));
    ASSERT_STREQ(dataType2->getName().c_str(), "dataType");
}

TEST_F(SerializationTest, TypedefDataType) {
    auto refDt = findDataType("uint32_t");
    auto dataType1 = new TypedefDataType(context, nullptr, "dataType", refDt);
    boost::json::object data;
    dataType1->serialize(data);

    auto dataType2 = new TypedefDataType(context);
    dataType2->deserialize(data);

    ASSERT_TRUE(cmp(dataType1, dataType2));
    ASSERT_TRUE(cmp(dataType2, "dataType = typedef uint32_t"));
}

TEST_F(SerializationTest, EnumDataType) {
    auto dataType1 = new EnumDataType(context, nullptr, "dataType", {
        std::make_pair(1, "one"),
        std::make_pair(2, "two"),
    });
    boost::json::object data;
    dataType1->serialize(data);

    auto dataType2 = new EnumDataType(context);
    dataType2->deserialize(data);

    ASSERT_TRUE(cmp(dataType1, dataType2));
    ASSERT_TRUE(cmp(dataType2, "dataType = enum { one = 1, two }"));
}

TEST_F(SerializationTest, StructureDataType) {
    auto symbolTable = new StandartSymbolTable(context);
    auto dataType1 = new StructureDataType(context, nullptr, "dataType", 100, symbolTable);
    boost::json::object data;
    dataType1->serialize(data);

    auto dataType2 = new StructureDataType(context);
    dataType2->deserialize(data);

    ASSERT_TRUE(cmp(dataType1, dataType2));
    ASSERT_STREQ(dataType2->getName().c_str(), "dataType");
}

TEST_F(SerializationTest, SignatureDataType) {
    auto callConv = std::make_shared<platform::FastcallCallingConvention>();
    auto returnDt = findDataType("uint32_t");
    auto dataType1 = new SignatureDataType(context, callConv, nullptr, "dataType", returnDt, {
        new FunctionParameterSymbol(context, nullptr, "param1", findDataType("uint32_t")),
        new FunctionParameterSymbol(context, nullptr, "param2", findDataType("uint32_t")),
    });
    boost::json::object data;
    dataType1->serialize(data);

    auto dataType2 = new SignatureDataType(context, callConv);
    dataType2->deserialize(data);

    ASSERT_TRUE(cmp(dataType1, dataType2));
    ASSERT_TRUE(cmp(dataType2, "dataType = signature fastcall uint32_t(uint32_t param1, uint32_t param2)"));
}

TEST_F(SerializationTest, VariableSymbol) {
    auto symbol1 = new VariableSymbol(context, nullptr, "var", findDataType("uint32_t"));
    boost::json::object data;
    symbol1->serialize(data);

    auto symbol2 = new VariableSymbol(context);
    symbol2->deserialize(data);

    ASSERT_TRUE(cmp(symbol1, symbol2));
    ASSERT_STREQ(symbol2->getName().c_str(), "var");
}

TEST_F(SerializationTest, FunctionSymbol) {
    auto testCallConv = std::make_shared<CallingConventionMock>();
    auto signatureDt = new SignatureDataType(context, testCallConv);
    auto stackSymbolTable = new StandartSymbolTable(context);
    auto instrSymbolTable = new StandartSymbolTable(context);
    auto symbol1 = new FunctionSymbol(context, nullptr, "func", signatureDt, stackSymbolTable, instrSymbolTable);
    boost::json::object data;
    symbol1->serialize(data);

    auto symbol2 = new FunctionSymbol(context);
    symbol2->deserialize(data);

    ASSERT_TRUE(cmp(symbol1, symbol2));
    ASSERT_STREQ(symbol2->getName().c_str(), "func");
}

TEST_F(SerializationTest, FunctionParameterSymbol) {
    auto symbol1 = new FunctionParameterSymbol(context, nullptr, "param", findDataType("uint32_t"));
    boost::json::object data;
    symbol1->serialize(data);

    auto symbol2 = new FunctionParameterSymbol(context);
    symbol2->deserialize(data);

    ASSERT_TRUE(cmp(symbol1, symbol2));
    ASSERT_STREQ(symbol2->getName().c_str(), "param");
}

TEST_F(SerializationTest, StructureFieldSymbol) {
    auto symbol1 = new StructureFieldSymbol(context, nullptr, "field", findDataType("uint32_t"));
    boost::json::object data;
    symbol1->serialize(data);

    auto symbol2 = new StructureFieldSymbol(context);
    symbol2->deserialize(data);

    ASSERT_TRUE(cmp(symbol1, symbol2));
    ASSERT_STREQ(symbol2->getName().c_str(), "field");
}

TEST_F(SerializationTest, StandartSymbolTable) {
    auto symbolTable1 = new StandartSymbolTable(context, nullptr, "symbolTable");
    symbolTable1->addSymbol(0x100,
        new VariableSymbol(context, nullptr, "var1", findDataType("uint32_t")));
    symbolTable1->addSymbol(0x104,
        new VariableSymbol(context, nullptr, "var2", findDataType("uint64_t")));

    boost::json::object data;
    symbolTable1->serialize(data);

    auto symbolTable2 = new StandartSymbolTable(context);
    symbolTable2->deserialize(data);

    ASSERT_TRUE(cmp(symbolTable1, symbolTable2));
    ASSERT_TRUE(cmp(symbolTable2, "symbolTable = { uint32_t var1 = 0x100, uint64_t var2 }"));
}

TEST_F(SerializationTest, OptimizedSymbolTable) {
    const size_t MinOffset = 0;
    const size_t MaxOffset = 1024;
    const size_t FragmentSize = 4;

    auto symbolTable1 = new OptimizedSymbolTable(context, nullptr, "symbolTable", MinOffset, MaxOffset, FragmentSize);
    symbolTable1->addSymbol(100,
        new VariableSymbol(context, nullptr, "var1", findDataType("uint32_t")));
    symbolTable1->addSymbol(800,
        new VariableSymbol(context, nullptr, "var2", findDataType("uint64_t")));

    boost::json::object data;
    symbolTable1->serialize(data);

    auto symbolTable2 = new OptimizedSymbolTable(context);
    symbolTable2->deserialize(data);

    ASSERT_TRUE(cmp(symbolTable1, symbolTable2));
    ASSERT_TRUE(cmp(symbolTable2, "symbolTable = { uint32_t var1 = 0x64, uint64_t var2 = 0x320 }"));
}