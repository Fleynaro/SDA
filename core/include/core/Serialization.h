#pragma once
#include <list>
#include <string>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

namespace sda
{
    // Interface for objects that can be serialized
    class ISerializable
    {
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive&, unsigned) {}

    public:
        struct KeyValue {
            std::string name;
            enum {
                Int,
                Float,
                Text
            } type;
            std::string value;
        };

        // Get the list of key values
        virtual std::list<KeyValue> getKeyValues() const { return {}; }
    };

    BOOST_SERIALIZATION_ASSUME_ABSTRACT(ISerializable)

    // Serialize object to stream
    void Serialize(const ISerializable* obj, std::ostream& os);

    // Deserialize object from stream
    ISerializable* Deserialize(std::istream& is);
};