#pragma once
#include <ostream>
#include <iomanip>

namespace utils
{
    // Outputs a hexadecimal number to the stream
    struct to_hex {
        bool leadingZeroes;

        to_hex(bool leadingZeroes = false);
    };

    std::ostream& operator<<(std::ostream& stream, const to_hex& toHex);
};