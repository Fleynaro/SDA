#pragma once
#include <vector>

namespace utils
{
    class BitSet
    {
        using Block = size_t;
        std::vector<Block> m_blocks;
    public:
        BitSet() = default;

        bool get(size_t index) const;

        void set(size_t index, bool value);

        void clear();

        BitSet operator|(const BitSet& other) const;

        bool operator==(const BitSet& other) const;

    private:
        Block getBlock(size_t blockIdx) const;

        size_t toBlockIdx(size_t index) const;

        size_t toBlockOffset(size_t index) const;
    };
};