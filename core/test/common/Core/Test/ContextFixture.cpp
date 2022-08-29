#include "ContextFixture.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/DataType/DataType.h"
#include "Core/DataType/DataTypeParser.h"
#include "Core/DataType/DataTypePrinter.h"
#include "Core/Symbol/Symbol.h"
#include "Core/SymbolTable/SymbolTable.h"
#include "Core/SymbolTable/SymbolTableParser.h"
#include "Platform/x86/Platform.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void ContextFixture::SetUp() {
    auto platform = std::make_unique<PlatformX86>(true);
    context = new Context(std::move(platform));
    context->initDefault();
}

void ContextFixture::TearDown() {
    delete context;
}

DataType* ContextFixture::findDataType(const std::string& name) const {
    return context->getDataTypes()->getByName(name);
}

DataType* ContextFixture::parseDataType(const std::string& text, bool withName) const {
    return DataTypeParser::Parse(text, context, withName);
}

DataType* ContextFixture::newTestStruct() const {
    return parseDataType("\
        TestStruct = struct { \
            uint32_t a, \
            float b, \
            uint64_t c, \
            int64_t d \
        } \
    ");
}

SymbolTable* ContextFixture::parseSymbolTable(const std::string& text, bool withName) const {
    return SymbolTableParser::Parse(text, context, false, withName);
}