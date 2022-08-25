#pragma once
#include <gtest/gtest.h>
#include "Core/Serialization.h"

namespace sda::test
{
    const std::list<std::string> ContextObjectExcludeFields = { "uuid", "name", "comment" };
    const std::list<std::string> ObjectExcludeFields = { "uuid" };

    ::testing::AssertionResult Compare(
        ISerializable* object1,
        ISerializable* object2);

    ::testing::AssertionResult CompareDeeply(
        ISerializable* object1,
        ISerializable* object2,
        const std::list<std::string>& excludedFields = {});

    ::testing::AssertionResult CompareDeeply(
        boost::json::object object1,
        boost::json::object object2,
        const std::list<std::string>& excludedFields = {});

    // static bool CompareDeeply(
    //     const boost::json::value& value1,
    //     const boost::json::value& value2,
    //     const std::list<std::string>& excludedFields = CompareExcludedFields);
};