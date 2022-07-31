#pragma once

namespace sda
{
    const size_t BitsInBytes = 8;
    const size_t MaxMaskSizeInBytes = 8;
    const size_t MaxMaskSizeInBits = MaxMaskSizeInBytes * BitsInBytes;

    class BitMask
    {
    public:
        using Value = size_t;

    private:
        Value m_mask;

    public:
        BitMask(Value mask);

        BitMask(size_t size, size_t offset);

        size_t getMaxSizeInBits() const;

        size_t getOffset() const;

        operator size_t() const;
    };
};