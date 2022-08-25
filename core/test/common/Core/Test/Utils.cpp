#include "Utils.h"

using namespace sda;
using namespace sda::test;

::testing::AssertionResult sda::test::Compare(
    ISerializable* object1,
    ISerializable* object2)
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
    ISerializable* object1,
    ISerializable* object2,
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

// bool sda::test::CompareDeeply(
//     const boost::json::value& value1,
//     const boost::json::value& value2,
//     const std::list<std::string>& excludedFields)
// {
//     if (value1.kind() != value2.kind())
//         return false;
//     if (value1.if_object()) {
//         auto& object1 = value1.as_object();
//         auto& object2 = value2.as_object();
//         if (object1.size() != object2.size())
//             return false;
//         for (auto it1 = object1.begin(); it1 != object1.end(); ++it1) {
//             if (std::find(excludedFields.begin(), excludedFields.end(), it1->key()) != excludedFields.end())
//                 continue;
//             auto it2 = object2.find(it1->key());
//             if (it2 == object2.end())
//                 return false;
//             if (!Compare(it1->value(), it2->value(), excludedFields))
//                 return false;
//         }
//     } else if (value1.if_array()) {
//         auto& array1 = value1.as_array();
//         auto& array2 = value2.as_array();
//         if (array1.size() != array2.size())
//             return false;
//         for (size_t i = 0; i < array1.size(); ++i) {
//             if (!Compare(array1[i], array2[i], excludedFields))
//                 return false;
//         }
//     } else {
//         return value1 == value2;
//     }
//     return true;
// }