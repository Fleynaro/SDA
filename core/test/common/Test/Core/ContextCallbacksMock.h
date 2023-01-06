#pragma once
#include <gmock/gmock.h>
#include "SDA/Core/Context.h"

namespace sda::test
{
    class ContextCallbacksMock : public Context::Callbacks
    {
    public:
        MOCK_METHOD(void, onObjectAdded, (Object* obj), ());

        MOCK_METHOD(void, onObjectModified, (Object* obj), ());

        MOCK_METHOD(void, onObjectRemoved, (Object* obj), ());
    };
};