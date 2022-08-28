#include "ContextFixture.h"
#include "Core/Image/AddressSpace.h"
#include "Core/Image/Image.h"
#include "Core/DataType/DataType.h"
#include "Core/DataType/DataTypeParser.h"
#include "Core/DataType/DataTypePrinter.h"
#include "Core/Symbol/Symbol.h"
#include "Core/SymbolTable/SymbolTable.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void ContextFixture::SetUp() {
    auto platform = std::make_unique<PlatformMock>();
    Mock::AllowLeak(platform.get());
    context = new Context(std::move(platform));
    context->initDefault();
}

void ContextFixture::TearDown() {
    delete context;
}

PlatformMock* ContextFixture::getPlatform() const {
    return dynamic_cast<PlatformMock*>(context->getPlatform());
}

DataType* ContextFixture::findDataType(const std::string& name) const {
    return context->getDataTypes()->getByName(name);
}

DataType* ContextFixture::parseDataType(const std::string& text) const {
    return DataTypeParser::ParseSingle(text, context);
}

DataType* ContextFixture::newTestStruct() const {
    return parseDataType("\
        struct { \
            uint32_t a, \
            float b, \
            uint64_t c, \
            int64_t d \
        } \
    ");
}