#include "Core/Utils/IOManip.h"

using namespace utils;

to_hex::to_hex(bool leadingZeroes)
    : leadingZeroes(leadingZeroes)
{}

std::ostream& utils::operator<<(std::ostream& stream, const to_hex& toHex) {
    stream << std::hex;
    if (toHex.leadingZeroes)
        stream << std::setfill('0') << std::setw(16) << std::right;
    return stream;
}