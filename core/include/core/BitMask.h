#pragma once

namespace sda
{
    class BitMask
    {
        size_t m_mask;
    public:
        BitMask(size_t mask);

        size_t getMask() const;
    };
};