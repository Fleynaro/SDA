#include "Database/Database.h"
#include "Database/Schema.h"
#include "Database/Transaction.h"
#include "Database/Loader.h"
#include "Factory.h"
#include "Core/DataType/DataTypeParser.h"
#include "Core/DataType/DataTypePrinter.h"
#include "Core/Test/ContextFixture.h"
#include "Core/Test/Utils/TestAssertion.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ContextDatabaseTest : public ContextFixture
{
    class ContextCallbacks : public Context::Callbacks
    {
        ContextDatabaseTest* test;
    public:
        ContextCallbacks(ContextDatabaseTest* test)
            : test(test)
        {}

        void onObjectAdded(Object* obj) override {
            test->transaction->markAsNew(obj);
        }

        void onObjectModified(Object* obj) override {
            test->transaction->markAsModified(obj);
        }

        void onObjectRemoved(Object* obj) override {
            test->transaction->markAsRemoved(obj);
        }
    };

    Database* database;
    Transaction* transaction;
protected:

    void SetUp() override {
        ContextFixture::SetUp();
        std::filesystem::remove("test.db");
        database = new Database("test.db", GetSchema());
        database->init();
        transaction = new Transaction(database);
        context->setCallbacks(std::make_shared<ContextCallbacks>(this));
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