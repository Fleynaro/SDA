#include "ContextFixture.h"
#include "SDA/Core/DataType/DataTypeParser.h"
#include "SDA/Core/DataType/DataTypePrinter.h"
#include "SDA/Core/SymbolTable/SymbolTableParser.h"
#include "SDA/Platform/x86/Platform.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void ContextFixture::SetUp() {
    platform = new PlatformX86(true);
    context = newContext();
    context->initDefault();
    globalSymbolTable = parseSymbolTable("\
        GlobalSymbolTable = {} \
    ");
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

FunctionSymbol* ContextFixture::newFunction(
    Offset offset,
    const std::string& name,
    const std::string& signature,
    SymbolTable* stackSymbolTable,
    SymbolTable* instrSymbolTable
) {
    if (!stackSymbolTable) {
        stackSymbolTable = parseSymbolTable("\
            StackSymbolTable = {} \
        ");
    }
    if (!instrSymbolTable) {
        instrSymbolTable = parseSymbolTable("\
            InstrSymbolTable = {} \
        ");
    }
    auto functionSignature = dynamic_cast<SignatureDataType*>(parseDataType(signature));
    auto functionSymbol = new FunctionSymbol(
        context, nullptr, name, functionSignature, stackSymbolTable, instrSymbolTable);
    globalSymbolTable->addSymbol(offset, functionSymbol);
    return functionSymbol;
}
