#include "ContextFixture.h"
#include "SDA/Core/DataType/DataTypePrinter.h"
#include "SDA/Core/SymbolTable/SymbolTableParser.h"
#include "SDA/Core/Utils/Logger.h"
#include "SDA/Platform/x86/Platform.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

void ContextFixture::SetUp() {
    utils::InitLogger();
    eventPipe = EventPipe::New();
    platform = new PlatformX86(true);
    context = newContext(eventPipe);
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

Context* ContextFixture::newContext(std::shared_ptr<EventPipe> eventPipe) {
    auto context = new Context(eventPipe, platform);
    createdContexts.push_back(context);
    return context;
}

DataType* ContextFixture::findDataType(const std::string& name) const {
    return context->getDataTypes()->getByName(name);
}

DataType* ContextFixture::parseDataType(const std::string& text) const {
    return parseDataTypes(text).back().dataType;
}

std::list<ParsedDataType> ContextFixture::parseDataTypes(const std::string& text) const {
    return DataTypeParser::Parse(text, context);
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

SymbolTable* ContextFixture::parseSymbolTable(const std::string& text, bool withName, SymbolTable* symbolTable) const {
    return SymbolTableParser::Parse(text, context, false, withName, symbolTable, false);
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

::testing::AssertionResult ContextFixture::cmpDataType(DataType* dataType, const std::string& expectedCode, bool withName) const {
    return cmpDataTypes({ { dataType, false } }, expectedCode, withName);
}

::testing::AssertionResult ContextFixture::cmpDataTypes(const std::list<ParsedDataType>& parsedDataTypes, const std::string& expectedCode, bool withName) const {
    auto actualCode = DataTypePrinter::Print(parsedDataTypes, context, withName);
    return Compare(actualCode, expectedCode);
}
