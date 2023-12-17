#include "SDA/Core/SymbolTable/SymbolTableParser.h"
#include "SDA/Core/SymbolTable/SymbolTablePrinter.h"
#include "Test/Core/ContextFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;

class SymbolTableParserTest : public ContextFixture
{
protected:
    SymbolTable* parse(const std::string& text, bool isStruct = false, sda::SymbolTable *symbolTable = nullptr) {
        return SymbolTableParser::Parse(text, context, isStruct, true, symbolTable);
    }

    std::string print(SymbolTable* symbolTable) const {
        return SymbolTablePrinter::Print(symbolTable, context, true);
    }
};

TEST_F(SymbolTableParserTest, Sample1) {
    auto expectedCode = "\
        // test symbol table \n\
        symbolTable = { \
            uint32_t a, \
            float b, \
            // some comment \n\
            uint64_t c = 0x100, \
            int64_t d = 0x200 \
        } \
    ";
    auto symbolTable = parse(expectedCode);
    auto actualCode = print(symbolTable);
    ASSERT_TRUE(Compare(actualCode, expectedCode));
}

TEST_F(SymbolTableParserTest, Sample2) {
    auto expectedCode = "\
        symbolTable = { \
            uint32_t a \
        } \
    ";
    auto symbolTable = parse(expectedCode);
    ASSERT_TRUE(Compare(print(symbolTable), expectedCode));
    // add field b to the symbol table
    auto expectedCode2 = "\
        symbolTable = { \
            uint32_t a, \
            float b \
        } \
    ";
    parse(expectedCode2, false, symbolTable);
    ASSERT_TRUE(Compare(print(symbolTable), expectedCode2));
    // remove field a from the symbol table
    auto expectedCode3 = "\
        symbolTable = { \
            float b = 0x4 \
        } \
    ";
    parse(expectedCode3, false, symbolTable);
    ASSERT_TRUE(Compare(print(symbolTable), expectedCode3));
}