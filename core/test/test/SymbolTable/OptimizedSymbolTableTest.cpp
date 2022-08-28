#include "Core/SymbolTable/OptimizedSymbolTable.h"
#include "Core/SymbolTable/StandartSymbolTable.h"
#include "Core/Symbol/VariableSymbol.h"
#include "Core/Test/ContextFixture.h"
#include "Core/Test/Utils.h"

using namespace sda;
using namespace sda::test;

class OptimizedSymbolTableTest : public ContextFixture
{
protected:
    OptimizedSymbolTable* symbolTable;
    std::vector<StandartSymbolTable*> fragmentTables;
    static const inline size_t MinOffset = 0;
    static const inline size_t MaxOffset = 1024;
    static const inline size_t FragmentSize = 4;

    void SetUp() override {
        ContextFixture::SetUp();
        symbolTable = new OptimizedSymbolTable(context, nullptr, "", MinOffset, MaxOffset, FragmentSize);
        fragmentTables = symbolTable->getSymbolTables();
    }
    
    void TearDown() override {
        delete symbolTable;
        ContextFixture::TearDown();
    }
};

TEST_F(OptimizedSymbolTableTest, GetSymbolAt) {
    auto var1 = new VariableSymbol(context, nullptr, "var1", findDataType("uint32_t"));
    symbolTable->addSymbol(200, var1); // should added to first table
    auto var2 = new VariableSymbol(context, nullptr, "var2", findDataType("uint64_t"));
    symbolTable->addSymbol(255, var2); // should added to first table (at boundary)
    auto var3 = new VariableSymbol(context, nullptr, "var3", findDataType("uint64_t"));
    symbolTable->addSymbol(512, var3); // should added to third table
    auto var4 = new VariableSymbol(context, nullptr, "var4", newTestStruct());
    symbolTable->addSymbol(1023, var4); // should added to last table (at boundary)

    SymbolTable::SymbolInfo symbolInfo;

    // empty
    {
        symbolInfo = symbolTable->getSymbolAt(100);
        ASSERT_FALSE(symbolInfo.symbol);
    }

    // var1
    {
        symbolInfo = symbolTable->getSymbolAt(200);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var1));
        ASSERT_TRUE(Compare(symbolInfo.symbolTable, fragmentTables[0]));
        ASSERT_EQ(symbolInfo.symbolOffset, 200);

        symbolInfo = symbolTable->getSymbolAt(201);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var1));
        ASSERT_TRUE(Compare(symbolInfo.symbolTable, fragmentTables[0]));
        ASSERT_EQ(symbolInfo.symbolOffset, 200);

        symbolInfo = symbolTable->getSymbolAt(203);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var1));
        ASSERT_TRUE(Compare(symbolInfo.symbolTable, fragmentTables[0]));
        ASSERT_EQ(symbolInfo.symbolOffset, 200);
    }

    // var 2
    {
        symbolInfo = symbolTable->getSymbolAt(255);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var2));
        ASSERT_TRUE(Compare(symbolInfo.symbolTable, fragmentTables[0]));
        ASSERT_EQ(symbolInfo.symbolOffset, 255);

        symbolInfo = symbolTable->getSymbolAt(256);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var2));
        ASSERT_TRUE(Compare(symbolInfo.symbolTable, fragmentTables[0]));
        ASSERT_EQ(symbolInfo.symbolOffset, 255);
    }

    // var 3
    {
        symbolInfo = symbolTable->getSymbolAt(514);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var3));
        ASSERT_TRUE(Compare(symbolInfo.symbolTable, fragmentTables[2]));
        ASSERT_EQ(symbolInfo.symbolOffset, 512);
    }

    // empty
    {
        symbolInfo = symbolTable->getSymbolAt(700);
        ASSERT_FALSE(symbolInfo.symbol);
    }

    // var 4
    {
        symbolInfo = symbolTable->getSymbolAt(1023);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var4));
        ASSERT_TRUE(Compare(symbolInfo.symbolTable, fragmentTables[3]));
        ASSERT_EQ(symbolInfo.symbolOffset, 1023);

        symbolInfo = symbolTable->getSymbolAt(1028);
        ASSERT_TRUE(Compare(symbolInfo.symbol, var4));
        ASSERT_TRUE(Compare(symbolInfo.symbolTable, fragmentTables[3]));
        ASSERT_EQ(symbolInfo.symbolOffset, 1023);
    }
}

TEST_F(OptimizedSymbolTableTest, OverlaySymbol) {
    symbolTable->addSymbol(255, // should added to first table (at boundary)
        new VariableSymbol(context, nullptr, "var1", findDataType("uint32_t")));
    ASSERT_THROW(
        symbolTable->addSymbol(256,
            new VariableSymbol(context, nullptr, "var2", findDataType("uint32_t")));
    , std::runtime_error);
}

TEST_F(OptimizedSymbolTableTest, RemoveSymbol) {
    ASSERT_EQ(symbolTable->getAllSymbols().size(), 0);
    symbolTable->addSymbol(200, // should added to first table
        new VariableSymbol(context, nullptr, "var1", findDataType("uint32_t")));
    symbolTable->addSymbol(255, // should added to first table (at boundary)
        new VariableSymbol(context, nullptr, "var2", findDataType("uint32_t")));
    symbolTable->addSymbol(512, // should added to third table
        new VariableSymbol(context, nullptr, "var3", findDataType("uint32_t")));
    ASSERT_EQ(symbolTable->getAllSymbols().size(), 3);
    symbolTable->removeSymbol(200);
    symbolTable->removeSymbol(255);
    symbolTable->removeSymbol(512);
    ASSERT_EQ(symbolTable->getAllSymbols().size(), 0);
}