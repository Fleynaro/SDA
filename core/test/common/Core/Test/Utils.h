#pragma once
#include <gtest/gtest.h>
#include "Core/Serialization.h"

namespace sda::test
{
    using CleanFunc = std::function<bool(char)>;
    const CleanFunc DefaultCleanFunc = [](char c) { return std::isspace(c); };
    const std::list<std::string> ContextObjectExcludeFields = { "uuid", "name", "comment" };
    const std::list<std::string> ObjectExcludeFields = { "uuid" };

    ::testing::AssertionResult Compare(
        const std::string& str1,
        const std::string& str2,
        const CleanFunc& cleanFunc = DefaultCleanFunc);

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
};