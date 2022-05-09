#pragma once
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
        virtual ~ISerializable() = default;
    };

    BOOST_SERIALIZATION_ASSUME_ABSTRACT(ISerializable)

    // Serialize object to stream
    void Serialize(const ISerializable* obj, std::ostream& os);

    // Deserialize object from stream
    ISerializable* Deserialize(std::istream& is);
};