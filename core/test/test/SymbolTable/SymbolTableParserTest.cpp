#include "Core/SymbolTable/SymbolTableParser.h"
#include "Core/SymbolTable/SymbolTablePrinter.h"
#include "Core/Test/ContextFixture.h"
#include "Core/Test/Utils.h"

using namespace sda;
using namespace sda::test;

class SymbolTableParserTest : public ContextFixture
{
protected:
    SymbolTable* parse(const std::string& text, bool isStruct = false) {
        return SymbolTableParser::Parse(text, context, isStruct, true);
    }

    std::string print(SymbolTable* symbolTable) const {
        return SymbolTablePrinter::Print(symbolTable, context, true);
    }
};

TEST_F(SymbolTableParserTest, Sample1) {
    auto expectedCode = "\
        ['test symbol table'] \
        symbolTable = { \
            uint32_t a, \
            float b, \
            uint64_t c = 0x100, \
            int64_t d = 0x200 \
        } \
    ";
    auto symbolTable = parse(expectedCode);
    auto actualCode = print(symbolTable);
    ASSERT_TRUE(Compare(actualCode, expectedCode));
}