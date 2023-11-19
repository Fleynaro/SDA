#include "SDA/Core/SymbolTable/StandartSymbolTable.h"
#include "SDA/Core/Symbol/VariableSymbol.h"
#include "Test/Core/ContextFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;

class StandartSymbolTableTest : public ContextFixture
{
protected:
    StandartSymbolTable* symbolTable;

    void SetUp() override {
        ContextFixture::SetUp();
        symbolTable = new StandartSymbolTable(context);
    }
};

TEST_F(StandartSymbolTableTest, GetSymbolAt) {
    auto var1 = new VariableSymbol(context, nullptr, "var1", findDataType("uint32_t"));
    symbolTable->addSymbol(0x104, var1);
    auto var2 = new VariableSymbol(context, nullptr, "var2", findDataType("uint64_t"));
    symbolTable->addSymbol(0x108, var2);
    auto var3 = new VariableSymbol(context, nullptr, "var3", newTestStruct());
    symbolTable->addSymbol(0x120, var3);

    SymbolTable::SymbolInfo symbolInfo;

    // empty
    {
        symbolInfo = symbolTable->getSymbolAt(0x100);
        ASSERT_FALSE(symbolInfo.symbol);
    }

    // var1
    {
        symbolInfo = symbolTable->getSymbolAt(0x104);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var1));
        ASSERT_EQ(symbolInfo.symbol->getOffset(), 0x104);

        symbolInfo = symbolTable->getSymbolAt(0x105);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var1)); 
        ASSERT_EQ(symbolInfo.symbol->getOffset(), 0x104);

        symbolInfo = symbolTable->getSymbolAt(0x107);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var1)); 
        ASSERT_EQ(symbolInfo.symbol->getOffset(), 0x104);
    }

    // var 2
    {
        symbolInfo = symbolTable->getSymbolAt(0x108);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var2));
        ASSERT_EQ(symbolInfo.symbol->getOffset(), 0x108);
    }

    // empty
    {
        symbolInfo = symbolTable->getSymbolAt(0x110);
        ASSERT_FALSE(symbolInfo.symbol);
    }

    // var 3
    {
        symbolInfo = symbolTable->getSymbolAt(0x120);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var3));
        ASSERT_EQ(symbolInfo.symbol->getOffset(), 0x120);

        symbolInfo = symbolTable->getSymbolAt(0x130);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var3));
        ASSERT_EQ(symbolInfo.symbol->getOffset(), 0x120);
    }
}

TEST_F(StandartSymbolTableTest, GetAllSymbolsRecursivelyAt) {
    auto testStructureCode = "\
        TestStruct = struct { \
            float a = 0x10, \
            uint32_t b = 0x20, \
        } \
    ";
    auto globalSymbolTableCode = "\
        { \
            TestStruct globalVar_0x200 = 0x200 \
        } \
    ";
    parseDataType(testStructureCode);
    auto symbolTable = parseSymbolTable(globalSymbolTableCode, false);

    // empty
    {
        auto symbols = symbolTable->getAllSymbolsRecursivelyAt(0x100);
        ASSERT_EQ(symbols.size(), 0);
    }

    // get globalVar_0x200
    {
        auto symbols = symbolTable->getAllSymbolsRecursivelyAt(0x200);
        ASSERT_EQ(symbols.size(), 1);
        ASSERT_EQ(symbols.front().symbol->getName(), "globalVar_0x200");
    }

    // get globalVar_0x200->a
    {
        auto symbols = symbolTable->getAllSymbolsRecursivelyAt(0x200 + 0x10);
        ASSERT_EQ(symbols.size(), 2);
        ASSERT_EQ(symbols.front().symbol->getName(), "globalVar_0x200");
        ASSERT_EQ(symbols.front().requestedOffset, 0x200 + 0x10);
        ASSERT_EQ(symbols.back().symbol->getName(), "a");
        ASSERT_EQ(symbols.back().requestedOffset, 0x10);
    }

    // get globalVar_0x200 with empty symbol
    {
        auto symbols = symbolTable->getAllSymbolsRecursivelyAt(0x200 + 0x5, true);
        ASSERT_EQ(symbols.size(), 2);
        ASSERT_EQ(symbols.front().symbol->getName(), "globalVar_0x200");
        ASSERT_EQ(symbols.front().requestedOffset, 0x200 + 0x5);
        ASSERT_EQ(symbols.back().symbol, nullptr);
        ASSERT_EQ(symbols.back().requestedOffset, 0x5);
    }
}

TEST_F(StandartSymbolTableTest, RemoveSymbol) {
    ASSERT_EQ(symbolTable->getAllSymbols().size(), 0);
    symbolTable->addSymbol(0x100,
        new VariableSymbol(context, nullptr, "var1", findDataType("uint32_t")));
    ASSERT_EQ(symbolTable->getAllSymbols().size(), 1);
    symbolTable->removeSymbol(0x100);
    ASSERT_EQ(symbolTable->getAllSymbols().size(), 0);
}