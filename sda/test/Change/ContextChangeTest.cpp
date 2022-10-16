#include "SDA/Change.h"
#include "SDA/Factory.h"
#include "SDA/Core/DataType/DataTypeParser.h"
#include "Test/Core/ContextFixture.h"

using namespace sda;
using namespace sda::test;
using namespace ::testing;

class ContextChangeTest : public ContextFixture
{
    class ContextCallbacks : public Context::Callbacks
    {
        ContextChangeTest* test;
    public:
        ContextCallbacks(ContextChangeTest* test)
            : test(test)
        {}

        void onObjectAdded(Object* obj) override {
            getOrCreateObjectChange()->markAsNew(obj);
        }

        void onObjectModified(Object* obj) override {
            getOrCreateObjectChange()->markAsModified(obj);
        }

        void onObjectRemoved(Object* obj) override {
            getOrCreateObjectChange()->markAsRemoved(obj);
        }

    private:
        ObjectChange* getOrCreateObjectChange() const {
            return test->changeChain->getChangeList()->getOrCreateObjectChange(test->factory);
        }
    };

    Factory* factory;
protected:
    ChangeChain* changeChain;

    void SetUp() override {
        ContextFixture::SetUp();
        changeChain = new ChangeChain();
        factory = new Factory(context);
        context->setCallbacks(std::make_shared<ContextCallbacks>(this));
    }
        
    void TearDown() override {
        delete factory;
        delete changeChain;
        ContextFixture::TearDown();
    }
};

TEST_F(ContextChangeTest, ObjectChange1) {
    // create a new object
    changeChain->newChangeList();
    auto enumDt = parseDataType("\
        enumDt = enum { \
            A, \
            B = 10, \
            C \
        } \
    ");
    ASSERT_STREQ(enumDt->getName().c_str(), "enumDt");

    // make the first change
    changeChain->newChangeList();
    enumDt->setName("enumDtRenamed1");
    ASSERT_STREQ(enumDt->getName().c_str(), "enumDtRenamed1");

    // make the second change
    changeChain->newChangeList();
    enumDt->setName("enumDtRenamed2");
    ASSERT_STREQ(enumDt->getName().c_str(), "enumDtRenamed2");

    // undo the second change
    changeChain->undo();
    ASSERT_STREQ(enumDt->getName().c_str(), "enumDtRenamed1");

    // undo the first change
    changeChain->undo();
    ASSERT_STREQ(enumDt->getName().c_str(), "enumDt");
}