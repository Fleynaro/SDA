#include "Core/SymbolTable/SymbolTableParser.h"
#include "Core/Test/ContextFixture.h"
#include "Core/Test/Utils.h"

using namespace sda;
using namespace sda::test;

class SymbolTableParserTest : public ContextFixture
{
protected:
    SymbolTable* parse(const std::string& text, bool isStruct = false) {
        return SymbolTableParser::Parse(text, context, isStruct);
    }
};

TEST_F(SymbolTableParserTest, Sample1) {
    auto symbolTable = parse("\
        symbolTable = { \
            uint32_t a, \
            float b, \
            uint64_t c = 0x100, \
            int64_t d = 20000 \
        } \
    ");
}