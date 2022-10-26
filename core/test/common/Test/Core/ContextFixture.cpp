#include "ContextFixture.h"
#include "SDA/Core/Image/AddressSpace.h"
#include "SDA/Core/Image/Image.h"
#include "SDA/Core/DataType/DataType.h"
#include "SDA/Core/DataType/DataTypeParser.h"
#include "SDA/Core/DataType/DataTypePrinter.h"
#include "SDA/Core/Symbol/Symbol.h"
#include "SDA/Core/SymbolTable/SymbolTable.h"
#include "SDA/Core/SymbolTable/SymbolTableParser.h"
#include "SDA/Platform/x86/Platform.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void ContextFixture::SetUp() {
    platform = new PlatformX86(true);
    context = newContext();
    context->initDefault();
}

void ContextFixture::TearDown() {
    for (auto ctx : createdContexts)
        delete ctx;
    delete platform;
}

Context* ContextFixture::newContext() {
    auto context = new Context(platform);
    createdContexts.push_back(context);
    return context;
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