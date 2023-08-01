#pragma once
#include <vector>

namespace utils
{
    class BitSet
    {
        using Block = size_t;
        std::vector<Block> m_blocks;
        bool m_inverted = false;
        size_t m_size = 0;
    public:
        BitSet() = default;

        bool get(size_t index) const;

        void set(size_t index, bool value);

        void clear();

        size_t size() const;

        BitSet operator|(const BitSet& other) const;

        BitSet operator&(const BitSet& other) const;

        BitSet operator~() const;

        bool operator==(const BitSet& other) const;

    private:
        Block getBlock(size_t blockIdx) const;

        size_t toBlockIdx(size_t index) const;

        size_t toBlockOffset(size_t index) const;

        void calculateSize();
    };
};