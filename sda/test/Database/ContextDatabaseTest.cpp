#include "SDA/Database/Database.h"
#include "SDA/Database/Schema.h"
#include "SDA/Database/Transaction.h"
#include "SDA/Database/Loader.h"
#include "SDA/Factory.h"
#include "SDA/Core/DataType/DataTypeParser.h"
#include "SDA/Core/DataType/DataTypePrinter.h"
#include "Test/Core/ContextFixture.h"
#include "Test/Core/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ContextDatabaseTest : public ContextFixture
{
    Database* database;
    Transaction* transaction;
protected:

    void SetUp() override {
        ContextFixture::SetUp();
        std::filesystem::remove("test.db");
        database = new Database("test.db", GetSchema());
        database->init();
        transaction = new Transaction(database);
        context->setCallbacks(std::make_shared<TransactionContextCallbacks>(transaction));
    }
        
    void TearDown() override {
        delete transaction;
        delete database;
        ContextFixture::TearDown();
    }

    void commit() {
        transaction->commit();
    }

    void loadToAnotherContext(Context* ctx) {
        Factory factory(ctx);
        Loader loader(database, &factory);
        loader.load();
    }

    ::testing::AssertionResult compareDataType(DataType* dataType, const std::string& expectedCode) const {
        auto actualCode = DataTypePrinter::Print(dataType, context, true);
        return Compare(actualCode, expectedCode);
    }
};

TEST_F(ContextDatabaseTest, ObjectSave1) {
    // create a new object and save (commit) it
    auto expectedCode = "\
        ['test data type'] \
        enumDt = enum { \
            A, \
            B = 10, \
            C \
        } \
    ";
    auto srcDataType = parseDataType(expectedCode);
    commit();

    // change the object and save it
    srcDataType->setName("myEnumDt");
    commit();
    // cancel the change above and save it
    srcDataType->setName("enumDt");
    commit();

    // load the object to another context
    auto anotherCtx = newContext();
    loadToAnotherContext(anotherCtx);
    // check if the object is loaded correctly
    auto loadedDataType = anotherCtx->getDataTypes()->getByName("enumDt");
    ASSERT_NE(loadedDataType, nullptr);
    ASSERT_TRUE(compareDataType(loadedDataType, expectedCode));
}