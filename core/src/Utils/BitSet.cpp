#include "SDA/Core/Utils/BitSet.h"

using namespace utils;

bool BitSet::get(size_t index) const {
    auto blockIdx = toBlockIdx(index);
    auto block = getBlock(blockIdx);
    auto offset = toBlockOffset(index);
    return (block & (Block(1) << offset)) != 0;
}

void BitSet::set(size_t index, bool value) {
    if (get(index) != value) {
        if (value) m_size++;
        else m_size--;
    }
    auto blockIdx = toBlockIdx(index);
    auto offset = toBlockOffset(index);
    if (blockIdx >= m_blocks.size()) {
        m_blocks.resize(blockIdx + 1);
    }
    if (value) {
        m_blocks[blockIdx] |= (Block(1) << offset);
    } else {
        m_blocks[blockIdx] &= ~(Block(1) << offset);
    }
}

void BitSet::clear() {
    m_blocks.clear();
}

size_t BitSet::size() const {
    return m_size;
}

BitSet BitSet::operator|(const BitSet& other) const {
    BitSet result;
    auto size = std::max(m_blocks.size(), other.m_blocks.size());
    result.m_blocks.resize(size);
    result.m_inverted = m_inverted || other.m_inverted;
    for (size_t i = 0; i < size; ++i) {
        auto block1 = getBlock(i);
        auto block2 = other.getBlock(i);
        result.m_blocks[i] = block1 | block2;
    }
    result.calculateSize();
    return result;
}

BitSet BitSet::operator&(const BitSet& other) const {
    BitSet result;
    auto size = std::max(m_blocks.size(), other.m_blocks.size());
    result.m_blocks.resize(size);
    result.m_inverted = m_inverted && other.m_inverted;
    for (size_t i = 0; i < size; ++i) {
        auto block1 = getBlock(i);
        auto block2 = other.getBlock(i);
        result.m_blocks[i] = block1 & block2;
    }
    result.calculateSize();
    return result;
}

BitSet BitSet::operator~() const {
    BitSet result;
    result.m_blocks.resize(m_blocks.size());
    result.m_inverted = !m_inverted;
    for (size_t i = 0; i < m_blocks.size(); ++i) {
        auto block = getBlock(i);
        result.m_blocks[i] = ~block;
    }
    result.calculateSize();
    return result;
}

bool BitSet::operator==(const BitSet& other) const {
    auto size = std::max(m_blocks.size(), other.m_blocks.size());
    for (size_t i = 0; i < size; ++i) {
        auto block1 = getBlock(i);
        auto block2 = other.getBlock(i);
        if (block1 != block2) {
            return false;
        }
    }
    return true;
}

BitSet::Block BitSet::getBlock(size_t blockIdx) const {
    if (blockIdx >= m_blocks.size()) {
        return Block(m_inverted ? size_t(-1) : 0);
    }
    return m_blocks[blockIdx];
}

size_t BitSet::toBlockIdx(size_t index) const {
    return index / (sizeof(Block) * 8);
}

size_t BitSet::toBlockOffset(size_t index) const {
    return index % (sizeof(Block) * 8);
}

void BitSet::calculateSize() {
    m_size = 0;
    for (auto block : m_blocks) {
        if (block) {
            for (size_t i = 0; i < sizeof(Block) * 8; ++i) {
                if ((block & (Block(1) << i)) != 0) {
                    m_size++;
                }
            }
        } else {
            if (m_inverted) {
                m_size += sizeof(Block) * 8;
            }
        }
    }
}
