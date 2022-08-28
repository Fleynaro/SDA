#include "Core/SymbolTable/StandartSymbolTable.h"
#include "Core/Symbol/VariableSymbol.h"
#include "Core/Test/ContextFixture.h"
#include "Core/Test/Utils.h"

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
        ASSERT_EQ(symbolInfo.symbolOffset, 0x104);

        symbolInfo = symbolTable->getSymbolAt(0x105);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var1)); 
        ASSERT_EQ(symbolInfo.symbolOffset, 0x104);

        symbolInfo = symbolTable->getSymbolAt(0x107);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var1)); 
        ASSERT_EQ(symbolInfo.symbolOffset, 0x104);
    }

    // var 2
    {
        symbolInfo = symbolTable->getSymbolAt(0x108);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var2));
        ASSERT_EQ(symbolInfo.symbolOffset, 0x108);
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
        ASSERT_EQ(symbolInfo.symbolOffset, 0x120);

        symbolInfo = symbolTable->getSymbolAt(0x130);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var3));
        ASSERT_EQ(symbolInfo.symbolOffset, 0x120);
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