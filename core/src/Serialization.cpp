#include "Core/Serialization.h"
#include "Core/Function.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace sda;

// All serializable objects must be defined here
BOOST_CLASS_EXPORT(Function)

void sda::Serialize(const ISerializable* obj, std::ostream& os) {
    boost::archive::text_oarchive oa(os);
    oa << obj;
}

ISerializable* sda::Deserialize(std::istream& is) {
    boost::archive::text_iarchive ia(is);
    ISerializable* obj;
    ia >> obj;
    return obj;
}