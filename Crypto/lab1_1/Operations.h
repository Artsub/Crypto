#pragma once
#include<vector>
#include"DESConfig.h"
inline std::vector<uint8_t> permuteBits(const std::vector<uint8_t>& data, const std::vector<uint16_t>& pBlock, bool reverseBitOrder = false, bool  isOneIndexed = true) {
    std::vector<uint8_t> result((pBlock.size() + 7) / 8);
    int position, blockIndex, bitOffset, resOffset, resIndex;

    for (int dataBitIndex = 0; dataBitIndex < pBlock.size(); dataBitIndex++) {
        position = pBlock[dataBitIndex] - (isOneIndexed ? 1 : 0);
        blockIndex = position / 8;

        bitOffset = reverseBitOrder ? position % 8 : 7 - position % 8;
        resOffset = 7 - dataBitIndex % 8;

        resIndex = dataBitIndex / 8;

        bool value = static_cast<uint8_t>((data[blockIndex] & (1 << bitOffset)));
        result[resIndex] = static_cast<uint8_t>(value
            ? result[resIndex] | (1 << resOffset)
            : result[resIndex] & ~(1 << resOffset));

    }

    return result;
}

inline std::vector<uint8_t> xorBits(std::vector<uint8_t> x, std::vector<uint8_t> y) {

    size_t size = std::min(x.size(), y.size());
    std::vector<uint8_t> res(size);
    for (int i = 0; i < size; i++) {
        res[i] = x[i] ^ y[i];
    }

    return res;
}

inline std::vector<uint8_t> substitution(std::vector<uint8_t>& data) {
    if (data.size() != 6)
        throw std::exception("key isnt 6 bytes");
    std::vector<uint8_t> result(4);
    uint64_t tmpBlock = 0;
    for (auto b : data) {
        tmpBlock = (b & 0xFF) | (tmpBlock << 8);
    }
    for (int i = 0; i < 8; i++) {
        std::vector<int> bitsArr(6);
        int sixBits = (int)((tmpBlock >> (6 * (8 - i - 1))) & 0xFF);
        for (int j = 0; j < 6; j++) {
            bitsArr[j] = (sixBits >> (5 - j)) & 1;
        }

        int row = (bitsArr[0] << 1) | bitsArr[5];
        int col = (bitsArr[1] << 3) | (bitsArr[2] << 2) | (bitsArr[3] << 1) | (bitsArr[4]);
        int value = S_BLOCKS[i][row][col];

        result[i / 2] |= uint8_t(((i & 1) != 0 ? value : value << 4));
    }
    return result;
}

inline uint32_t leftCycleShift(int shift, int value, int size) {
    return ((value << shift) | (value >> (size - shift))) & ((1 << size) - 1);
}

inline uint32_t ComputeMask(uint32_t x) {
    uint32_t m = 0;
    for (uint32_t i = 0; i < 32; i++)
    {
        if ((x & (1u << (int)i)) != 0)
        {
            m |= 0xFFFFFFFFu << (int)i;
            break;
        }
    }

    return m;
}

inline uint32_t LeftRotate(uint32_t val, uint32_t shift)
{
    return (val << shift) | (val >> (32 - shift));
}

inline uint32_t RightRotate(uint32_t val, uint32_t shift)
{
    return (val >> shift) | (val << (32 - shift));
}

inline uint32_t toUInt32(const std::vector<uint8_t>& bytes, size_t index) {
    

    return static_cast<uint32_t>(bytes[index]) |
        (static_cast<uint32_t>(bytes[index + 1]) << 8) |
        (static_cast<uint32_t>(bytes[index + 2]) << 16) |
        (static_cast<uint32_t>(bytes[index + 3]) << 24);
}

inline std::vector<uint8_t> uint64ToBytes(uint64_t val) {
    std::vector<uint8_t> bytes(8);
    for (int i = 7; i >= 0; --i) {
        bytes[i] = static_cast<uint8_t>(val & 0xFF);
        val >>= 8;
    }
    return bytes;
}

inline uint64_t bytesToUint64(const std::vector<uint8_t>& bytes) {
    uint64_t val = 0;
    for (int i = 0; i < 8; ++i) {
        val = (val << 8) | bytes[i];
    }
    return val;
}