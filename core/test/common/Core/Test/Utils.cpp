#include "Utils.h"

using namespace sda;
using namespace sda::test;

::testing::AssertionResult sda::test::Compare(
    const std::string& str1,
    const std::string& str2,
    const CleanFunc& cleanFunc)
{
    auto cleanStr1 = str1;
    auto cleanStr2 = str2;
    for (auto cleanStr : { &cleanStr1, &cleanStr2 }) {
        auto it = std::remove_if(cleanStr->begin(), cleanStr->end(), cleanFunc);
        cleanStr->erase(it, cleanStr->end());
    }
    if (cleanStr1 != cleanStr2)
        return ::testing::AssertionFailure() << "\"" << cleanStr1 << "\" != \"" << cleanStr2 << "\"";
    return ::testing::AssertionSuccess();
}

::testing::AssertionResult sda::test::Compare(
    utils::ISerializable* object1,
    utils::ISerializable* object2)
{
    if (object1 == object2)
        return ::testing::AssertionSuccess();
    if (object1 == nullptr || object2 == nullptr)
        return ::testing::AssertionFailure() << "One of objects is nullptr";
    boost::json::object data1;
    object1->serialize(data1);
    boost::json::object data2;
    object2->serialize(data2);
    return ::testing::AssertionFailure() << std::endl
        << "Objects are not equal by pointer:" << std::endl
        << "object1 = " << data1 << std::endl
        << "object2 = " << data2;
}

::testing::AssertionResult sda::test::CompareDeeply(
    utils::ISerializable* object1,
    utils::ISerializable* object2,
    const std::list<std::string>& excludedFields)
{
    if (object1 == nullptr && object2 == nullptr)
        return ::testing::AssertionSuccess();
    if (object1 == nullptr || object2 == nullptr)
        return ::testing::AssertionFailure() << "One of objects is nullptr";
    boost::json::object data1;
    object1->serialize(data1);
    boost::json::object data2;
    object2->serialize(data2);
    return CompareDeeply(data1, data2, excludedFields);
}

::testing::AssertionResult sda::test::CompareDeeply(
    boost::json::object object1,
    boost::json::object object2,
    const std::list<std::string>& excludedFields)
{
    for (auto& field : excludedFields) {
        object1.erase(field);
        object2.erase(field);
    }
    if (object1 == object2)
        return ::testing::AssertionSuccess();
    return ::testing::AssertionFailure() << std::endl
        << "Objects are not equal:" << std::endl
        << "object1 = " << object1 << std::endl
        << "object2 = " << object2;
}