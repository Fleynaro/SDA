#include "SDA/Core/Utils/String.h"
#include <iomanip>

using namespace utils;

std::string utils::ToHex(size_t value, bool leadingZeroes) {
    std::stringstream ss;
    ss << std::hex;
    if (leadingZeroes)
        ss << std::setfill('0') << std::setw(16) << std::right;
    ss << value;
    return ss.str();
}
