#pragma once
#include <boost/json.hpp>

namespace utils
{
    template<typename T>
    T get_number(const boost::json::value& value) {
        if (value.is_int64()) {
            auto n = value.get_int64();
            return reinterpret_cast<T&>(n);
        }
        else if (value.is_uint64()) {
            auto n = value.get_uint64();
            return reinterpret_cast<T&>(n);
        }
        else {
            throw std::runtime_error("Invalid number type");
        }
    }
};